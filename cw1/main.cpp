#include <cstddef>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <random>

#include <parlay/parallel.h>
#include <parlay/primitives.h>
#include <parlay/sequence.h>
#include <parlay/slice.h>

const size_t N = 100'000'000;
const size_t BLOCK = 1'000;
const size_t TESTS = 5;

template<typename It>
std::pair<It, It> partition(It left, It right) {
  int pivot = *(left + (right - left) / 2);

  It m1 = std::partition(left, right, [&](const auto& x) { return x < pivot; });
  It m2 = std::partition(m1, right, [&](const auto& x) { return x <= pivot; });

  return {m1, m2};
}

template<typename It>
void seq_quicksort(It left, It right) {
    if ((right - left) <= 1) return;

    auto [m1, m2] = partition(left, right);

    seq_quicksort(left, m1);
    seq_quicksort(m2, right);
}

double measure_seq(const std::vector<int>& data, const std::vector<int>& target) {
    std::vector<int> data_copy = data;

    auto t0 = std::chrono::high_resolution_clock::now();
    seq_quicksort(data_copy.begin(), data_copy.end());
    auto t1 = std::chrono::high_resolution_clock::now();

    if (data_copy != target)
        std::cout << "SEQ VALIDATION ERROR\n";

    return std::chrono::duration<double, std::milli>(t1 - t0).count();
}

template<typename It>
void par_quicksort(It left, It right) {
    if ((right - left) < BLOCK) {
        seq_quicksort(left, right);
        return;
    }

    auto pivots = partition(left, right);
    It m1 = pivots.first;
    It m2 = pivots.second;

    parlay::par_do(
        [&] { par_quicksort(left, m1); },
        [&] { par_quicksort(m2, right); }
    );
}

double measure_par(const std::vector<int>& data, const std::vector<int>& target) {
    auto data_copy = parlay::to_sequence(data);

    auto t0 = std::chrono::high_resolution_clock::now();
    par_quicksort(data_copy.begin(), data_copy.end());
    auto t1 = std::chrono::high_resolution_clock::now();

    bool ok = true;
    for (size_t i = 0; i < data.size(); i++) {
        if (data_copy[i] != target[i]) { ok = false; break; }
    }
    if (!ok) std::cout << "PAR VALIDATION ERROR\n";

    return std::chrono::duration<double, std::milli>(t1 - t0).count();
}

struct TestCase {
    std::string name;
    std::vector<int> input;
    std::vector<int> expected;
};

std::vector<TestCase> testCases = {
    {
        "empty",
        {},
        {}
    },
    {
        "single_element",
        {42},
        {42}
    },
    {
        "already_sorted",
        {1, 2, 3, 4, 5},
        {1, 2, 3, 4, 5}
    },
    {
        "reverse_sorted",
        {9, 7, 5, 3, 1},
        {1, 3, 5, 7, 9}
    },
    {
        "with_duplicates",
        {5, 3, 7, 3, 9, 5, 1},
        {1, 3, 3, 5, 5, 7, 9}
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
    std::vector<int> seq = tc.input;
    seq_quicksort(seq.begin(), seq.end());
    if (seq != tc.expected) {
        std::cout << "[FAIL][SEQ] " << tc.name
            << "  actual: " << vec2str(seq)
            << "  expected: " << vec2str(tc.expected) << "\n";
        return false;
    }

    auto par_seq = parlay::to_sequence(tc.input);
    par_quicksort(par_seq.begin(), par_seq.end());

    std::vector<int> par_vec(par_seq.begin(), par_seq.end());
    if (par_vec != tc.expected) {
        std::cout << "[FAIL][PAR] " << tc.name
            << "  actual: " << vec2str(par_vec)
            << "  expected: " << vec2str(tc.expected) << "\n";
        return false;
    }

    std::cout << "[PASS] " << tc.name << "\n";
    return true;
}


int main() {
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(INT32_MIN, INT32_MAX);

    for (const auto& tc : testCases) {
        if (!run_test(tc))  return 1;
    }

    double sum_seq = 0;
    double sum_par = 0;

    for (size_t test = 1; test <= TESTS; test++) {
        std::cout << "\n--- TEST " << test << " ---\n";

        std::vector<int> data(N);
        for (size_t i = 0; i < N; i++) data[i] = dist(rng);

        std::vector<int> target = data;
        std::sort(target.begin(), target.end());

        double t_seq = measure_seq(data, target);
        double t_par = measure_par(data, target);

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
    std::cout << "AVERAGE SPEEDUP: " << average_seq / average_par << "\n";
    std::cout << "--------------------------\n";

    return 0;
}
