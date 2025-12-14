#include <cstddef>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <chrono>
#include <random>
#include <atomic>
#include <limits>
#include <cstdint>

#include <parlay/parallel.h>
#include <parlay/primitives.h>
#include <parlay/sequence.h>
#include <parlay/slice.h>

using vertex = uint32_t;
static constexpr vertex EMPTY = std::numeric_limits<vertex>::max();

const size_t SIDE  = 300;
const size_t TESTS = 5;

inline vertex id3(size_t x, size_t y, size_t z, size_t side) {
  return static_cast<vertex>((x * side + y) * side + z);
}

static std::vector<std::vector<vertex>> make_cube_adj(size_t side) {
  size_t n = side * side * side;
  std::vector<std::vector<vertex>> adj(n);
  for (size_t x = 0; x < side; ++x) {
    for (size_t y = 0; y < side; ++y) {
      for (size_t z = 0; z < side; ++z) {
        vertex v = id3(x, y, z, side);
        if (x > 0)        adj[v].push_back(id3(x - 1, y, z, side));
        if (x + 1 < side) adj[v].push_back(id3(x + 1, y, z, side));
        if (y > 0)        adj[v].push_back(id3(x, y - 1, z, side));
        if (y + 1 < side) adj[v].push_back(id3(x, y + 1, z, side));
        if (z > 0)        adj[v].push_back(id3(x, y, z - 1, side));
        if (z + 1 < side) adj[v].push_back(id3(x, y, z + 1, side));
      }
    }
  }
  return adj;
}

static std::vector<int32_t> seq_bfs(const std::vector<std::vector<vertex>>& adj, vertex s) {
  size_t n = adj.size();
  std::vector<int32_t> dist(n, -1);

  std::vector<vertex> q(n);
  size_t head = 0, tail = 0;

  dist[s] = 0;
  q[tail++] = s;

  while (head < tail) {
    vertex v = q[head++];
    int32_t nd = dist[v] + 1;
    const auto &neighbors = adj[v];
    for (size_t i = 0; i < neighbors.size(); ++i) {
      vertex u = neighbors[i];
      if (dist[u] == -1) {
        dist[u] = nd;
        q[tail++] = u;
      }
    }
  }
  return dist;
}

static std::vector<int32_t> par_bfs(const std::vector<std::vector<vertex>>& adj, vertex s) {
  size_t n = adj.size();

  std::vector<std::atomic<uint8_t>> visited(n);
  parlay::parallel_for(0, n, [&](size_t i) {
    visited[i].store(0, std::memory_order_relaxed);
  });

  std::vector<int32_t> dist(n);
  parlay::parallel_for(0, n, [&](size_t i) { dist[i] = -1; });

  visited[s].store(1, std::memory_order_relaxed);
  dist[s] = 0;

  parlay::sequence<vertex> frontier(1);
  frontier[0] = s;

  int32_t level = 0;

  while (frontier.size() != 0) {
    size_t m = frontier.size();

    parlay::sequence<size_t> deg(m);
    parlay::parallel_for(0, m, [&](size_t i) {
      deg[i] = adj[frontier[i]].size();
    });

    auto scanned = parlay::scan(deg);
    parlay::sequence<size_t> offset = std::move(scanned.first);
    size_t next_frontier_size = scanned.second;

    parlay::sequence<vertex> next_frontier(next_frontier_size);
    parlay::parallel_for(0, next_frontier_size, [&](size_t i) {
      next_frontier[i] = EMPTY;
    });

    parlay::parallel_for(0, m, [&](size_t idx) {
      vertex v = frontier[idx];
      size_t out = offset[idx];
      const auto &neighbors = adj[v];
      for (size_t j = 0; j < neighbors.size(); ++j) {
        vertex u = neighbors[j];
        uint8_t expected = 0;
        bool ok = visited[u].compare_exchange_strong(expected, 1, std::memory_order_relaxed);
        if (ok) {
          dist[u] = level + 1;
          next_frontier[out] = u;
        } else {
          next_frontier[out] = EMPTY;
        }
        out++;
      }
    });

    frontier = parlay::filter(next_frontier, [&](vertex x) { return x != EMPTY; });
    level++;
  }

  return dist;
}


static bool equal_dist(const std::vector<int32_t>& a, const std::vector<int32_t>& b) {
  if (a.size() != b.size()) return false;
  for (size_t i = 0; i < a.size(); i++) if (a[i] != b[i]) return false;
  return true;
}

static void test_path_graph() {
  const size_t n = 10000;
  std::vector<std::vector<vertex>> adj(n);
  for (size_t i = 0; i + 1 < n; i++) {
    adj[i].push_back((vertex)(i + 1));
    adj[i + 1].push_back((vertex)i);
  }

  auto d1 = seq_bfs(adj, 0);
  auto d2 = par_bfs(adj, 0);

  if (!equal_dist(d1, d2)) {
    std::cout << "TEST PATH: ERROR\n";
    std::exit(1);
  }
}

static void test_random_graph() {
  const size_t n = 5000;
  const size_t m = 20000;
  std::mt19937 rng(12345);
  std::uniform_int_distribution<uint32_t> uid(0, (uint32_t)n - 1);

  std::vector<std::vector<vertex>> adj(n);
  for (size_t i = 0; i < m; i++) {
    vertex u = uid(rng);
    vertex v = uid(rng);
    if (u == v) continue;
    adj[u].push_back(v);
    adj[v].push_back(u);
  }

  vertex s = uid(rng);
  auto d1 = seq_bfs(adj, s);
  auto d2 = par_bfs(adj, s);

  if (!equal_dist(d1, d2)) {
    std::cout << "TEST RANDOM: ERROR\n";
    std::exit(1);
  }
}

static void test_small_cube() {
  const size_t s = 30;
  auto adj = make_cube_adj(s);

  auto d1 = seq_bfs(adj, 0);
  auto d2 = par_bfs(adj, 0);

  if (!equal_dist(d1, d2)) {
    std::cout << "TEST CUBE: ERROR\n";
    std::exit(1);
  }

  for (size_t x = 0; x < s; x += 7) {
    for (size_t y = 0; y < s; y += 7) {
      for (size_t z = 0; z < s; z += 7) {
        vertex v = id3(x, y, z, s);
        int32_t want = static_cast<int32_t>(x + y + z);
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


static uint64_t checksum_sample(size_t side, const std::vector<int32_t>& dist) {
  uint64_t cs = 0;
  cs += static_cast<uint64_t>(dist[0]);
  cs += static_cast<uint64_t>(dist[id3(side - 1, 0, 0, side)]);
  cs += static_cast<uint64_t>(dist[id3(0, side - 1, 0, side)]);
  cs += static_cast<uint64_t>(dist[id3(0, 0, side - 1, side)]);
  cs += static_cast<uint64_t>(dist[id3(side - 1, side - 1, side - 1, side)]);
  return cs;
}

double measure_seq_cube(const std::vector<std::vector<vertex>>& adj, size_t side) {
  auto t0 = std::chrono::high_resolution_clock::now();
  auto dist = seq_bfs(adj, 0);
  auto t1 = std::chrono::high_resolution_clock::now();

  if (dist[id3(side - 1, side - 1, side - 1, side)] != static_cast<int32_t>(3 * (side - 1))) {
    std::cout << "SEQ VALIDATION ERROR\n";
  }

  return std::chrono::duration<double, std::milli>(t1 - t0).count();
}

double measure_par_cube(const std::vector<std::vector<vertex>>& adj, size_t side) {
  auto t0 = std::chrono::high_resolution_clock::now();
  auto dist = par_bfs(adj, 0);
  auto t1 = std::chrono::high_resolution_clock::now();

  if (dist[id3(side - 1, side - 1, side - 1, side)] != static_cast<int32_t>(3 * (side - 1))) {
    std::cout << "PAR VALIDATION ERROR\n";
  }

  volatile uint64_t cs = checksum_sample(side, dist);
  (void)cs;

  return std::chrono::duration<double, std::milli>(t1 - t0).count();
}

int main() {
  run_tests();

  auto adj = make_cube_adj(SIDE);
  size_t n = adj.size();

  double sum_seq = 0;
  double sum_par = 0;

  for (size_t test = 1; test <= TESTS; test++) {
    std::cout << "\n--- TEST " << test << " ---\n";

    double t_seq = measure_seq_cube(adj, SIDE);
    double t_par = measure_par_cube(adj, SIDE);

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
