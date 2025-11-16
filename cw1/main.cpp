#include <cstddef>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <iostream>
#include <chrono>

#include <parlay/parallel.h>
#include <parlay/primitives.h>
#include <parlay/sequence.h>
#include <parlay/slice.h>

const int array_size = 100'000'000;
// const int array_size = 10'000'000;
const int BLOCK = array_size / 4;
// const int BLOCK = 1 << 16;

// const size_t PAR_THRESHOLD = 1 << 16;

void seq_quicksort(std::vector<int>& data, int left, int right) {
    if (left >= right) return;
    int m2 = left, m1 = right, pivot = data[(left+right)/2];
    while (m2 <= m1) {
        while (data[m2] < pivot) ++m2;
        while (data[m1] > pivot) --m1;
        if (m2 <= m1) std::swap(data[m2++], data[m1--]);
    }
    seq_quicksort(data, left, m1);
    seq_quicksort(data, m2, right);
}

double measure_seq_quicksort(const std::vector<int>& data) {
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

    parlay::par_do(
        [&] { par_quicksort(parlay::make_slice(left), out.cut(0, m1));},
        [&] { par_quicksort(parlay::make_slice(right), out.cut(m2, n));});

    parlay::copy(middle, out.cut(m1, m2));
}

double measure_par_quicksort(const std::vector<int>& data) {
    size_t n = data.size();
    auto data_copy = parlay::to_sequence(data);
    auto result = parlay::sequence<int>(n);

    std::cout << "\n------ par_quicksort ------" << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    par_quicksort(data_copy.cut(0, n), result.cut(0, n));
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


    auto seq = measure_seq_quicksort(data);
    auto par = measure_par_quicksort(data);

    std::cout << "DIFFERENCE " << seq / par << " times" << std::endl;

    return 0;
}
