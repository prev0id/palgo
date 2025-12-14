#include <cstddef>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <chrono>
#include <random>
#include <atomic>

#include <parlay/parallel.h>
#include <parlay/primitives.h>
#include <parlay/sequence.h>
#include <parlay/slice.h>

using Vertex = int;
using Graph = std::vector<std::vector<Vertex>>;
static constexpr Vertex EMPTY = -1;

const size_t SIDE  = 300;
const size_t TESTS = 5;

inline Vertex id3(size_t x, size_t y, size_t z, size_t side) {
  return static_cast<Vertex>((x * side + y) * side + z);
}

Graph make_cube_graph(size_t side) {
    size_t n = side * side * side;
    Graph graph(n);
    for (size_t x = 0; x < side; ++x) {
        for (size_t y = 0; y < side; ++y) {
            for (size_t z = 0; z < side; ++z) {
                Vertex v = id3(x, y, z, side);
                if (x > 0)        graph[v].push_back(id3(x - 1, y, z, side));
                if (x + 1 < side) graph[v].push_back(id3(x + 1, y, z, side));
                if (y > 0)        graph[v].push_back(id3(x, y - 1, z, side));
                if (y + 1 < side) graph[v].push_back(id3(x, y + 1, z, side));
                if (z > 0)        graph[v].push_back(id3(x, y, z - 1, side));
                if (z + 1 < side) graph[v].push_back(id3(x, y, z + 1, side));
            }
        }
    }
    return graph;
}

std::vector<int> seq_bfs(const Graph& graph, Vertex s) {
    size_t n = graph.size();
    std::vector<int> dist(n, -1);
    std::vector<bool> visited(n, false);
    std::queue<Vertex> q;

    dist[s] = 0;
    q.push(s);
    visited[s] = true;

    while (!q.empty()) {
        Vertex u = q.front();
        q.pop();
        for (Vertex v : graph[u]) {
            if (visited[v]) {
                continue;
            }
            dist[v] = dist[u] + 1;
            visited[v] = true;
            q.push(v);
        }
    }
    return dist;
}

std::vector<int> par_bfs(const Graph& graph, Vertex s) {
    size_t n = graph.size();

    std::vector<std::atomic<bool>> visited(n);
    parlay::parallel_for(0, n, [&](size_t i) {
        visited[i].store(0, std::memory_order_relaxed);
    });

    std::vector<int> dist(n);

    visited[s].store(1, std::memory_order_relaxed);
    dist[s] = 0;

    parlay::sequence<Vertex> frontier(1);
    frontier[0] = s;

    int level = 1;

    while (frontier.size() != 0) {
        size_t m = frontier.size();

        parlay::sequence<size_t> deg(m);
        parlay::parallel_for(0, m, [&](size_t i) {
            deg[i] = graph[frontier[i]].size();
        });

        auto scanned = parlay::scan(deg);
        parlay::sequence<size_t> offsets = std::move(scanned.first);
        size_t next_frontier_size = scanned.second;

        parlay::sequence<Vertex> next_frontier(next_frontier_size);
        parlay::parallel_for(0, next_frontier_size, [&](size_t idx) {
            next_frontier[idx] = EMPTY;
        });

        parlay::parallel_for(0, m, [&](size_t idx) {
            Vertex v = frontier[idx];
            size_t offset = offsets[idx];
            const auto &neighbors = graph[v];
            // parlay::parallel_for(0, neighbors.size(), [&](size_t neighbor_idx) {
            for (size_t neighbor_idx = 0; neighbor_idx < neighbors.size(); neighbor_idx++) {
                Vertex neighbor = neighbors[neighbor_idx];
                bool expected = false;

                bool ok = visited[neighbor].compare_exchange_strong(expected, true, std::memory_order_relaxed);
                if (ok) {
                    dist[neighbor] = level;
                    next_frontier[offset + neighbor_idx] = neighbor;
                } else {
                    next_frontier[offset + neighbor_idx] = EMPTY;
                }
            // });
            }
        });

        frontier = parlay::filter(next_frontier, [&](Vertex x) { return x != EMPTY; });
        level++;
    }

    return dist;
}


bool equal_dist(const std::vector<int>& a, const std::vector<int>& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); i++) if (a[i] != b[i]) return false;
    return true;
}

void test_path_graph() {
    const size_t n = 10000;
    Graph graph(n);
    for (size_t i = 0; i + 1 < n; i++) {
        graph[i].push_back((Vertex)(i + 1));
        graph[i + 1].push_back((Vertex)i);
    }

    auto d1 = seq_bfs(graph, 0);
    auto d2 = par_bfs(graph, 0);

    if (!equal_dist(d1, d2)) {
        std::cout << "TEST PATH: ERROR\n";
        std::exit(1);
    }
}

 void test_random_graph() {
    const size_t n = 5000;
    const size_t m = 20000;
    std::mt19937 rng(12345);
    std::uniform_int_distribution<int> uid(0, (int)n - 1);

    Graph graph(n);
    for (size_t i = 0; i < m; i++) {
        Vertex u = uid(rng);
        Vertex v = uid(rng);
        if (u == v) continue;
        graph[u].push_back(v);
        graph[v].push_back(u);
    }

    Vertex s = uid(rng);
    auto d1 = seq_bfs(graph, s);
    auto d2 = par_bfs(graph, s);

    if (!equal_dist(d1, d2)) {
        std::cout << "TEST RANDOM: ERROR\n";
        std::exit(1);
    }
}

void test_small_cube() {
    const size_t s = 30;
    auto graph = make_cube_graph(s);

    auto d1 = seq_bfs(graph, 0);
    auto d2 = par_bfs(graph, 0);

    if (!equal_dist(d1, d2)) {
        std::cout << "TEST CUBE: ERROR\n";
        std::exit(1);
    }

    for (size_t x = 0; x < s; x += 7) {
        for (size_t y = 0; y < s; y += 7) {
            for (size_t z = 0; z < s; z += 7) {
                Vertex v = id3(x, y, z, s);
                int want = static_cast<int>(x + y + z);
                if (d1[v] != want) {
                    std::cout << "TEST CUBE FORMULA: ERROR\n";
                    std::exit(1);
                }
            }
        }
    }
}

static void run_tests() {
    test_path_graph();
    test_random_graph();
    test_small_cube();
    std::cout << "ALL TESTS PASSED\n";
}


double measure_seq_cube(const Graph& graph, size_t side) {
    auto t0 = std::chrono::high_resolution_clock::now();
    auto dist = seq_bfs(graph, 0);
    auto t1 = std::chrono::high_resolution_clock::now();

    if (dist[id3(side - 1, side - 1, side - 1, side)] != static_cast<int>(3 * (side - 1))) {
        std::cout << "SEQ VALIDATION ERROR\n";
    }

    return std::chrono::duration<double, std::milli>(t1 - t0).count();
}

double measure_par_cube(const Graph& graph, size_t side) {
    auto t0 = std::chrono::high_resolution_clock::now();
    auto dist = par_bfs(graph, 0);
    auto t1 = std::chrono::high_resolution_clock::now();

    if (dist[id3(side - 1, side - 1, side - 1, side)] != static_cast<int>(3 * (side - 1))) {
        std::cout << "PAR VALIDATION ERROR\n";
    }

    return std::chrono::duration<double, std::milli>(t1 - t0).count();
}

int main() {
    run_tests();

    auto graph = make_cube_graph(SIDE);
    size_t n = graph.size();

    double sum_seq = 0;
    double sum_par = 0;

    for (size_t test = 1; test <= TESTS; test++) {
        std::cout << "\n--- TEST " << test << " ---\n";

        double t_seq = measure_seq_cube(graph, SIDE);
        double t_par = measure_par_cube(graph, SIDE);

        sum_seq += t_seq;
        sum_par += t_par;

        std::cout << "SEQ: " << t_seq << " ms\n";
        std::cout << "PAR: " << t_par << " ms\n";
        std::cout << "Speedup: " << (t_seq / t_par) << "\n";
    }

    double average_seq = sum_seq / TESTS;
    double average_par = sum_par / TESTS;

    std::cout << "\n--------------------------\n";
    std::cout << "AVERAGE SEQ: " << average_seq << " ms\n";
    std::cout << "AVERAGE PAR: " << average_par << " ms\n";
    std::cout << "AVERAGE SPEEDUP: " << (average_seq / average_par) << "\n";
    std::cout << "--------------------------\n";

    return 0;
}
