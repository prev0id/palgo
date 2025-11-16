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

// const size_t N = 100'000'000;
const size_t N = 10'000'000;
const size_t PAR_THRESHOLD = N / 4;
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
    size_t n = src.size();
    if (n <= PAR_THRESHOLD) {
      parlay::copy(src, dst);
      std::sort(dst.begin(), dst.end());
      return;
    }

    int pivot = src[n / 2];

    // allocate flag arrays
    parlay::sequence<int> is_left(n);
    parlay::sequence<int> is_equal(n);

    // fill flags in parallel
    parlay::parallel_for(0, n, [&](size_t i) {
      int x = src[i];
      is_left[i]  = (x <  pivot) ? 1 : 0;
      is_equal[i] = (x == pivot) ? 1 : 0;
    });

    // scan returns pair<sequence, total>
    auto left_scan  = parlay::scan(is_left);
    auto equal_scan = parlay::scan(is_equal);

    parlay::sequence<int> pref_left  = std::move(left_scan.first);
    parlay::sequence<int> pref_equal = std::move(equal_scan.first);

    size_t cnt_left  = (size_t) left_scan.second;
    size_t cnt_equal = (size_t) equal_scan.second;
    // cnt_right = n - cnt_left - cnt_equal;

    // write elements into dst according to positions computed from prefix sums
    parlay::parallel_for(0, n, [&](size_t i) {
      int x = src[i];
      if (x < pivot) {
        size_t pos = (size_t)pref_left[i];
        dst[pos] = x;
      } else if (x == pivot) {
        size_t pos = cnt_left + (size_t)pref_equal[i];
        dst[pos] = x;
      } else {
        size_t pos = cnt_left + cnt_equal +
                     (i - (size_t)pref_left[i] - (size_t)pref_equal[i]);
        dst[pos] = x;
      }
    });

    // recursively sort left and right parts in parallel
    parlay::par_do(
      [&] {
        if (cnt_left > 0)
          par_quicksort(dst.cut(0, cnt_left), src.cut(0, cnt_left));
      },
      [&] {
        size_t right_off = cnt_left + cnt_equal;
        if (right_off < n)
          par_quicksort(dst.cut(right_off, n), src.cut(right_off, n));
      }
    );

    // copy sorted parts back into dst (middle already in place)
    if (cnt_left > 0)
      parlay::copy(src.cut(0, cnt_left), dst.cut(0, cnt_left));
    size_t right_off = cnt_left + cnt_equal;
    if (right_off < n)
      parlay::copy(src.cut(right_off, n), dst.cut(right_off, n));
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
    std::cout << "AVERAGE SEQ: " << average_par << " ms\n";
    std::cout << "AVERAGE PAR: " << average_par << " ms\n";
    std::cout << "AVERAGE SPEEDUP: " << average_seq / average_par << "\n";
    std::cout << "--------------------------\n";

    return 0;
}
