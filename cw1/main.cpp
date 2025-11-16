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
const size_t BLOCK = 16'400'000;
const size_t TESTS = 5;

void seq_quicksort(std::vector<int>& data, int left, int right) {
    if (left >= right) return;
    int m1 = left, m2 = right, pivot = data[(left+right)/2];
    while (m1 <= m2) {
        while (data[m1] < pivot) ++m1;
        while (data[m2] > pivot) --m2;
        if (m1 <= m2) std::swap(data[m1++], data[m2--]);
    }
    seq_quicksort(data, left, m2);
    seq_quicksort(data, m1, right);
}

double measure_seq(const std::vector<int>& data, const std::vector<int>& target) {
    std::vector<int> data_copy = data;

    auto t0 = std::chrono::high_resolution_clock::now();
    seq_quicksort(data_copy, 0, data_copy.size()-1);
    auto t1 = std::chrono::high_resolution_clock::now();

    if (data_copy != target)
        std::cout << "SEQ VALIDATION ERROR\n";

    return std::chrono::duration<double, std::milli>(t1 - t0).count();
}

void par_quicksort(parlay::slice<int*, int*> src, parlay::slice<int*, int*> dst) {
    long n = src.size();
    if (n < BLOCK) {
        parlay::copy(src, dst);
        std::sort(dst.begin(), dst.end());
        return;
    }
    int pivot = src[n / 2];

    auto left = parlay::filter(src, [&](int x) {return x < pivot;});
    auto middle = parlay::filter(src, [&](int x) {return x == pivot;});
    auto right = parlay::filter(src, [&](int x) {return x > pivot;});

    size_t m1 = left.size();
    size_t m2 = left.size() + middle.size();

    parlay::par_do(
        [&] { par_quicksort(parlay::make_slice(left), dst.cut(0, m1));},
        [&] { par_quicksort(parlay::make_slice(right), dst.cut(m2, n));});

    parlay::copy(middle, dst.cut(m1, m2));
}

double measure_par(const std::vector<int>& data, const std::vector<int>& target) {
    size_t n = data.size();

    auto data_copy = parlay::to_sequence(data);
    auto dst = parlay::sequence<int>(n);

    auto t0 = std::chrono::high_resolution_clock::now();
    par_quicksort(data_copy.cut(0, n), dst.cut(0, n));
    auto t1 = std::chrono::high_resolution_clock::now();

    bool ok = true;
    for (size_t i = 0; i < n; i++)
        if (dst[i] != target[i]) { ok = false; break; }

    if (!ok) std::cout << "PAR VALIDATION ERROR\n";

    return std::chrono::duration<double, std::milli>(t1 - t0).count();
}

int main() {
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(INT32_MIN, INT32_MAX);

    double sum_seq = 0;
    double sum_par = 0;

    for (int test = 1; test <= TESTS; test++) {
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
        std::cout << "Speedup: " << (t_seq / t_par) << std::endl;
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
