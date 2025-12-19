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
