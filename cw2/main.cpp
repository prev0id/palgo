#include <iostream>
#include <vector>
#include <queue>
#include <chrono>
#include <cstdint>

#include <parlay/parallel.h>
#include <parlay/primitives.h>
#include <parlay/sequence.h>
#include <parlay/delayed.h>

using Vertex = uint32_t;
using Graph = parlay::sequence<parlay::sequence<Vertex>>;

parlay::sequence<Vertex> seq_bfs(const Graph& G, Vertex src) {
    size_t n = G.size();
    parlay::sequence<Vertex> dist(n, Vertex(-1));
    std::queue<Vertex> q;

    dist[src] = 0;
    q.push(src);

    while (!q.empty()) {
        Vertex u = q.front(); q.pop();
        for (Vertex v : G[u]) {
            if (dist[v] == Vertex(-1)) {
                dist[v] = dist[u] + 1;
                q.push(v);
            }
        }
    }
    return dist;
}

// Параллельный BFS (top-down подход с параллельной обработкой фронта)
parlay::sequence<Vertex> par_bfs(const Graph& G, Vertex src) {
    size_t n = G.size();
    parlay::sequence<Vertex> dist(n, Vertex(-1));
    parlay::sequence<Vertex> frontier = parlay::sequence<Vertex>{src};
    dist[src] = 0;

    while (frontier.size() > 0) {
        auto next_frontier = parlay::delayed::map(frontier, [&](Vertex u) {
            return parlay::delayed::map(G[u], [&](Vertex v) {
                return (dist[v] == Vertex(-1)) ? v : Vertex(-1);
            });
        });

        auto candidates = parlay::flatten(next_frontier);
        auto valid = parlay::filter(candidates, [](Vertex v) { return v != Vertex(-1); });

        auto unique_next = parlay::remove_duplicates(parlay::unique(valid));

        size_t next_level = dist[frontier[0]] + 1;

        parlay::parallel_for(0, unique_next.size(), [&](size_t i) {
            Vertex v = unique_next[i];
            if (dist[v] == Vertex(-1)) {
                dist[v] = next_level;
            }
        });

        frontier = std::move(unique_next);
    }

    return dist;
}

// Генерация кубического графа size x size x size
Graph generate_cubic_graph(int side) {
    size_t n = side * side * side;
    Graph G(n);

    auto idx = [side](int x, int y, int z) -> Vertex {
        return Vertex(x * side * side + y * side + z);
    };

    parlay::parallel_for(0, n, [&](size_t i) {
        int x = i / (side * side);
        int y = (i / side) % side;
        int z = i % side;

        std::vector<Vertex> neighbors;
        neighbors.reserve(6);

        int dx[6] = {1, -1, 0, 0, 0, 0};
        int dy[6] = {0, 0, 1, -1, 0, 0};
        int dz[6] = {0, 0, 0, 0, 1, -1};

        for (int d = 0; d < 6; ++d) {
            int nx = x + dx[d], ny = y + dy[d], nz = z + dz[d];
            if (nx >= 0 && nx < side && ny >= 0 && ny < side && nz >= 0 && nz < side) {
                neighbors.push_back(idx(nx, ny, nz));
            }
        }
        G[i] = parlay::to_sequence(neighbors);
    });

    return G;
}

// Проверка корректности: сравнение результатов seq и par
bool validate(const parlay::sequence<Vertex>& d1, const parlay::sequence<Vertex>& d2) {
    if (d1.size() != d2.size()) return false;
    for (size_t i = 0; i < d1.size(); ++i) {
        if (d1[i] != d2[i]) {
            std::cout << "Mismatch at vertex " << i << ": seq=" << d1[i] << ", par=" << d2[i] << "\n";
            return false;
        }
    }
    return true;
}

// Измерение времени
double measure_seq(const Graph& G, Vertex src, parlay::sequence<Vertex>& reference) {
    auto t0 = std::chrono::high_resolution_clock::now();
    reference = seq_bfs(G, src);
    auto t1 = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(t1 - t0).count();
}

double measure_par(const Graph& G, Vertex src, const parlay::sequence<Vertex>& reference) {
    auto t0 = std::chrono::high_resolution_clock::now();
    auto result = par_bfs(G, src);
    auto t1 = std::chrono::high_resolution_clock::now();

    if (!validate(reference, result)) {
        std::cout << "PAR VALIDATION FAILED!\n";
    }

    return std::chrono::duration<double, std::milli>(t1 - t0).count();
}

int main() {
    const int SIDE = 100;
    const Vertex SRC = 0;
    const int TESTS = 5;

    Graph G = generate_cubic_graph(SIDE);

    double sum_seq = 0, sum_par = 0;

    parlay::sequence<Vertex> reference;

    for (int test = 1; test <= TESTS; ++test) {
        std::cout << "\n--- TEST " << test << " ---\n";

        double t_seq = measure_seq(G, SRC, reference);
        double t_par = measure_par(G, SRC, reference);

        sum_seq += t_seq;
        sum_par += t_par;

        std::cout << "SEQ time: " << t_seq << " ms\n";
        std::cout << "PAR time: " << t_par << " ms\n";
        std::cout << "Speedup:  " << (t_seq / t_par) << "x\n";
    }

    double avg_seq = sum_seq / TESTS;
    double avg_par = sum_par / TESTS;

    std::cout << "\n--------------------------\n";
    std::cout << "AVERAGE SEQ: " << avg_seq << " ms\n";
    std::cout << "AVERAGE PAR: " << avg_par << " ms\n";
    std::cout << "AVERAGE SPEEDUP: " << (avg_seq / avg_par) << "x\n";
    std::cout << "--------------------------\n";

    return 0;
}
