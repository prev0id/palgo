#include <cstddef>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <chrono>
#include <atomic>

#include <parlay/parallel.h>
#include <parlay/primitives.h>
#include <parlay/sequence.h>
#include <parlay/slice.h>

using Vertex = int;
using Graph = std::vector<std::vector<Vertex>>;
Vertex EMPTY = -1;
using clk = std::chrono::high_resolution_clock;
using ms = std::chrono::duration<double, std::milli>;

const size_t SIDE  = 300;
const size_t TESTS = 5;

Vertex make_vertex_id(size_t x, size_t y, size_t z, size_t side) {
  return (x * side + y) * side + z;
}

Graph make_cube_graph(size_t side) {
    size_t n = side * side * side;
    Graph graph(n);
    for (size_t x = 0; x < side; ++x) {
        for (size_t y = 0; y < side; ++y) {
            for (size_t z = 0; z < side; ++z) {
                Vertex v = make_vertex_id(x, y, z, side);
                if (x > 0)
                    graph[v].push_back(make_vertex_id(x - 1, y, z, side));
                if (x + 1 < side)
                    graph[v].push_back(make_vertex_id(x + 1, y, z, side));
                if (y > 0)
                    graph[v].push_back(make_vertex_id(x, y - 1, z, side));
                if (y + 1 < side)
                    graph[v].push_back(make_vertex_id(x, y + 1, z, side));
                if (z > 0)
                    graph[v].push_back(make_vertex_id(x, y, z - 1, side));
                if (z + 1 < side)
                    graph[v].push_back(make_vertex_id(x, y, z + 1, side));
            }
        }
    }
    return graph;
}

std::vector<int> seq_bfs(const Graph& graph, Vertex source) {
    size_t n = graph.size();
    if (n <= 0 || source < 0 || source >= n) {
        return std::vector<int>();
    }

    std::vector<int> dist(n, -1);
    dist[source] = 0;

    std::vector<bool> visited(n);
    visited[source] = true;

    std::queue<Vertex> queue;
    queue.push(source);

    while (!queue.empty()) {
        Vertex vertex = queue.front();
        queue.pop();
        for (Vertex neighbor : graph[vertex]) {
            if (visited[neighbor]) {
                continue;
            }
            dist[neighbor] = dist[vertex] + 1;
            visited[neighbor] = true;
            queue.push(neighbor);
        }
    }
    return dist;
}

std::vector<int> par_bfs(const Graph& graph, Vertex source) {
    size_t n = graph.size();

    if (n <= 0 || source < 0 || source >= n) {
        return std::vector<int>();
    }

    std::vector<std::atomic<bool>> visited(n);
    visited[source].store(true);

    parlay::sequence<int> dist(n, -1);
    dist[source] = 0;

    parlay::sequence<Vertex> frontier(1);
    frontier[0] = source;

    int level = 1;

    while (frontier.size() != 0) {
        size_t frontier_size = frontier.size();

        parlay::sequence<size_t> deg = parlay::tabulate(frontier_size, [&](size_t idx)-> size_t {
           return  graph[frontier[idx]].size();
        });

        auto scanned = parlay::scan(deg);
        parlay::sequence<size_t> offsets = scanned.first;
        size_t next_frontier_size = scanned.second;

        parlay::sequence<Vertex> next_frontier(next_frontier_size, EMPTY);

        parlay::parallel_for(0, frontier_size, [&](size_t idx) {
            Vertex vertex = frontier[idx];
            size_t offset = offsets[idx];
            const std::vector<Vertex> &neighbors = graph[vertex];
            for (size_t neighbor_idx = 0; neighbor_idx < neighbors.size(); neighbor_idx++) {
                Vertex neighbor = neighbors[neighbor_idx];
                bool expected = false;

                bool was_set = visited[neighbor].compare_exchange_strong(expected, true, std::memory_order_relaxed);
                if (was_set) {
                    dist[neighbor] = level;
                    next_frontier[offset + neighbor_idx] = neighbor;
                }
            }
        });

        frontier = parlay::filter(next_frontier, [&](Vertex x) { return x != EMPTY; });
        level++;
    }

    return dist.to_vector();
}

struct TestCase {
    std::string name;
    Graph graph;
    Vertex src;
    std::vector<int> expected;
};

const std::vector<TestCase> testCases = {
    {
        "empty_graph",
        {},
        0,
        {}
    },
    {
        "single_vertex",
        { {} },
        0,
        { 0 }
    },
    {
        "path_graph",
        { {1}, {0,2}, {1,3}, {2,4}, {3} },
        0,
        {0,1,2,3,4}
    },
    {
        "cycle_graph",
        { {1,3}, {0,2}, {1,3}, {2,0} },
        0,
        {0,1,2,1}
    },
    {
        "star_graph",
        { {1,2,3,4}, {0}, {0}, {0}, {0} },
        0,
        {0,1,1,1,1}
    },
    {
        "two_components",
        { {1}, {0}, {3}, {2} },
        0,
        {0,1,-1,-1}
    },
    {
        "self_loops",
        { {0,1}, {0,1,2}, {1,2} },
        0,
        {0,1,2}
    },
    {
        "complete_graph",
        { {1,2,3}, {0,2,3}, {0,1,3}, {0,1,2} },
        2,
        {1,1,0,1}
    },
};

std::string vec2str(const std::vector<int>& v) {
    std::string s = "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) s += ", ";
        s += std::to_string(v[i]);
    }
    s += "]";
    return s;
}

bool run_test(const TestCase& tc) {
    std::vector<int> seq_result = seq_bfs(tc.graph, tc.src);
    if (seq_result != tc.expected) {
        std::cout << "[FAIL][SEQ] " << tc.name << " actual "
            << vec2str(seq_result) << " expected "
            << vec2str(tc.expected) << "\n";
        return false;
    }

    std::vector<int> par_result = par_bfs(tc.graph, tc.src);
    if (par_result != tc.expected) {
        std::cout << "[FAIL][PAR] " << tc.name << " actual "
            << vec2str(par_result) << " expected "
            << vec2str(tc.expected) << "\n";
        return false;
    }
    std::cout << "[PASS] " << tc.name << "\n";
    return true;
}

bool test_cube() {
    Graph graph = make_cube_graph(SIDE);
    Vertex src = make_vertex_id(0,0,0,SIDE);

    std::vector<int> seq = seq_bfs(graph, src);
    std::vector<int> par = par_bfs(graph, src);

    for (size_t id = 0; id < graph.size(); ++id) {
        size_t x = id / (SIDE*SIDE);
        size_t y = (id % (SIDE*SIDE)) / SIDE;
        size_t z = id % SIDE;
        int expected = x + y + z;
        if (seq[id] != expected) {
            std::cout << "[FAIL][SEQ][CUBE] id=" << id << " got=" << seq[id] << " exp=" << expected << "\n";
            return false;
        }
        if (par[id] != expected) {
            std::cout << "[FAIL][PAR][CUBE] id=" << id << " got=" << par[id] << " exp=" << expected << "\n";
            return false;
        }
    }
    std::cout << "[PASS] small_cube (150^3)\n";
    return true;
}

bool test_long_path() {
    size_t n = 1'000'000;
    Graph g(n);
    for (size_t i = 0; i + 1 < n; ++i) {
        g[i].push_back(i+1);
        g[i+1].push_back(i);
    }
    const Vertex src = 0;

    std::vector<int> seq = seq_bfs(g, src);
    std::vector<int> par = par_bfs(g, src);

    for (size_t i = 0; i < n; ++i) {
        int expected = static_cast<int>(i);
        if (seq[i] != expected) {
            std::cout << "[FAIL][SEQ][PATH] i=" << i << " got=" << seq[i] << " exp=" << expected << "\n";
            return false;
        }
        if (par[i] != expected) {
            std::cout << "[FAIL][PAR][PATH] i=" << i << " got=" << par[i] << " exp=" << expected << "\n";
            return false;
        }
    }
    std::cout << "[PASS] long_path (1 000 000 vertices)\n";
    return true;
}

bool test_random_big() {
    const size_t n = 1'000'000;
    const size_t m = 4'000'000;

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(0, n-1);

    Graph graph(n);
    for (size_t i = 0; i < m; ++i) {
        size_t u = dist(rng);
        size_t v = dist(rng);
        graph[u].push_back(v);
        graph[v].push_back(u);
    }

    const Vertex src = 0;
    std::vector<int> seq = seq_bfs(graph, src);
    std::vector<int> par = par_bfs(graph, src);

    if (seq != par) {
        std::cout << "[FAIL][COMPARE][RANDOM] seq and par differ\n";
        return false;
    }
    std::cout << "[PASS] random_big (1 000 000 vertices, 2_000_000 edges)\n";
    return true;
}

bool test_big_star() {
    const size_t leaves = 2'000'000;
    const size_t n = leaves + 1;
    Graph graph(n);
    for (size_t i = 1; i < n; ++i) {
        graph[0].push_back(i);
        graph[i].push_back(0);
    }

    const Vertex src = 0;
    std::vector<int> seq = seq_bfs(graph, src);
    std::vector<int> par = par_bfs(graph, src);

    if (seq[0] != 0 || par[0] != 0) {
        std::cout << "[FAIL][STAR] center distance not zero\n";
        return false;
    }
    for (size_t i = 1; i < n; ++i) {
        if (seq[i] != 1) {
            std::cout << "[FAIL][SEQ][STAR] leaf " << i << " dist=" << seq[i] << "\n";
            return false;
        }
        if (par[i] != 1) {
            std::cout << "[FAIL][PAR][STAR] leaf " << i << " dist=" << par[i] << "\n";
            return false;
        }
    }
    std::cout << "[PASS] big_star (" << n << " vertices)\n";
    return true;
}


double measure_seq_cube(const Graph& graph, size_t side) {
    auto t0 = clk::now();
    auto dist = seq_bfs(graph, 0);
    auto t1 = clk::now();

    return ms(t1 - t0).count();
}

double measure_par_cube(const Graph& graph, size_t side) {
    auto t0 = clk::now();
    auto dist = par_bfs(graph, 0);
    auto t1 = clk::now();

    return ms(t1 - t0).count();
}

int main() {
    for (const auto& tc : testCases) {
        if (!run_test(tc)) return 1;
    }

    if (!test_cube()) return 1;
    if (!test_long_path()) return 1;
    if (!test_random_big()) return 1;
    if (!test_big_star()) return 1;

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
        std::cout << "SPEEDUP: " << (t_seq / t_par) << "\n";
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
