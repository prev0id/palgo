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
const size_t BLOCK = 10'000;
const size_t TESTS = 5;


template<typename It>
It partition(It left, It right) {
    int pivot = *(left + (right - left)/2);
    It middle = std::partition(left, right,
        [&](const auto& x) { return x < pivot; });
    std::partition(middle, right,
        [&](const auto& x) { return x <= pivot; });
    return middle;
}

template<typename It>
void seq_quicksort(It left, It right) {
    if ((right - left) <= 1) return;

    It middle = partition(left, right);

    seq_quicksort(left, middle);
    seq_quicksort(middle+1, right);
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
    long n = right - left;
    if (n < BLOCK) {
        seq_quicksort(left, right);
        return;
    }

    It middle = partition(left, right);

    parlay::par_do(
        [&] { par_quicksort(left, middle); },
        [&] { par_quicksort(middle, right); });
}

double measure_par(const std::vector<int>& data, const std::vector<int>& target) {
    auto data_copy = parlay::to_sequence(data);

    auto t0 = std::chrono::high_resolution_clock::now();
    par_quicksort(data_copy.begin(), data_copy.end());
    auto t1 = std::chrono::high_resolution_clock::now();

    bool ok = true;
    for (size_t i = 0; i < data.size(); i++)
        if (data_copy[i] != target[i]) { ok = false; break; }

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
