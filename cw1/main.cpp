#include <cstddef>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <iostream>
#include <chrono>

#include "include/include/parlay/parallel.h"
#include "include/include/parlay/primitives.h"
#include "include/include/parlay/sequence.h"
#include "include/include/parlay/slice.h"

/*
 * Господи, надеюсь это последний раз когда мне приходится использовать этот прекрасный язык и не менее прекрасный cmake
 */

const int array_size = 100'000'000;
// const int array_size = 10'000'000;
const int BLOCK = 10000;

void seq_quicksort(std::vector<int>& data, int left, int right) {
    if (left >= right) return;

    int pivot = data[(left + right) / 2];
    int i = left, j = right;
    while (i <= j) {
        while (data[i] < pivot) ++i;
        while (data[j] > pivot) --j;
        if (i <= j) {
            std::swap(data[i], data[j]);
            ++i; --j;
        }
    }
    if (left < j) seq_quicksort(data, left, j);
    if (i < right) seq_quicksort(data, i, right);
}

double mesure_seq_quicksort(const std::vector<int>& data) {
    auto result = std::vector(data);

    std::cout << "\n------ seq_quicksort ------" << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    seq_quicksort(result, 0, result.size() - 1);
    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "VALIDATION: ";
    if (std::is_sorted(result.begin(), result.end())) {
        std::cout << "OK\n";
    } else {
        std::cout << "ERROR\n";
    }

    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << "RESULT: " << duration.count() << " ms\n";
    std::cout << "---------------------------" <<  std::endl;
    return duration.count();
}

void par_quicksort(parlay::slice<int*, int*> data, parlay::slice<int*, int*> out) {
    long n = data.size();
    if (n < BLOCK) {
        parlay::copy(data, out);
        std::sort(out.begin(), out.end());
        return;
    }
    int pivot = data[n / 2];

    auto left = parlay::filter(data, [&](int x) {return x < pivot;});
    auto middle = parlay::filter(data, [&](int x) {return x == pivot;});
    auto right = parlay::filter(data, [&](int x) {return x > pivot;});

    size_t m1 = left.size();
    size_t m2 = left.size() + middle.size();

    // parlay::filter возвращает какую-то хрень, копируем из нее в data обратно
    // parlay::copy(left, data.cut(0, m1));
    // parlay::copy(right, data.cut(m2, n));

    parlay::par_do(
        [&] { par_quicksort(parlay::make_slice(left), out.cut(0, m1));},
        [&] { par_quicksort(parlay::make_slice(right), out.cut(m2, n));});

    parlay::copy(middle, out.cut(m1, m2));
}

template <typename Range, typename Less>
void qsort(Range in, Range out, Less less) {
    long n = in.size();
    if (n < 10000) {
        parlay::copy(in, out);
        std::sort(out.begin(), out.end(), less);
    } else {
        auto pivot = parlay::sort(parlay::tabulate(101, [&] (long i) {
        return in[i*n/101];}))[50];
        auto [x, offsets] = parlay::counting_sort(in, 3, [&] (auto k) {
        return less(k, pivot) ? 0u : less(pivot, k) ? 2u : 1u;});
        auto& split = x;
        long nl = offsets[1];
        long nm = offsets[2];
        parlay::copy(split.cut(nl,nm), out.cut(nl,nm));
        parlay::par_do(
            [&] { qsort(split.cut(0,nl), out.cut(0,nl), less);},
            [&] { qsort(split.cut(nm,n), in.cut(nm,n), less);});
    }
}

template <typename Range, typename Less = std::less<>>
auto quicksort(Range& in, Less less = {}) {
  long n = in.size();
  using T = typename Range::value_type;
  parlay::sequence<T> out(n);
  qsort(in.cut(0,n), out.cut(0,n), less);
  return out;
}

double mesure_par_quicksort(const std::vector<int>& data) {
    size_t n = data.size();
    auto data_copy = parlay::to_sequence(data);
    auto result = parlay::sequence<int>(n);

    std::cout << "\n------ par_quicksort ------" << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    quicksort(data_copy);
    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "VALIDATION: ";
    if (std::is_sorted(result.begin(), result.end())) {
        std::cout << "OK\n";
    } else {
        std::cout << "ERROR\n";
    }

    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << "RESULT: " << duration.count() << " ms\n";
    std::cout << "---------------------------" <<  std::endl;
    return duration.count();
}

int main() {
    std::srand(std::time(nullptr));

    std::cout << "Generating array len=" << array_size << std::endl;
    std::vector<int> data(array_size);
    std::generate(data.begin(), data.end(), []() {
        return std::rand();
    });
    std::cout << "Array created" << std::endl;


    auto seq = mesure_seq_quicksort(data);
    auto par = mesure_par_quicksort(data);

    std::cout << "DIFFERENCE " << seq / par << " times" << std::endl;

    return 0;
}
