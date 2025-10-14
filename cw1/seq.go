package main

// https://neerc.ifmo.ru/wiki/index.php?title=Быстрая_сортировка
func QuickSortSeq(data []int) {
	if len(data) <= 1 {
		return
	}
	m := partitionSeq(data)
	QuickSortSeq(data[:m+1])
	QuickSortSeq(data[m+1:])
}

func partitionSeq(data []int) int {
	l, r := 0, len(data)-1

	pivot := data[(l+r)/2]

	for l <= r {
		for data[l] < pivot {
			l++
		}
		for data[r] > pivot {
			r--
		}
		if l >= r {
			break
		}
		data[l], data[r] = data[r], data[l]
		l++
		r--
	}
	return r
}

func ExlusiveScanSec(data []int, f func(int, int) int) []int {
	result := make([]int, len(data))
	for idx := 1; idx < len(data); idx++ {
		result[idx] = f(result[idx-1], data[idx])
	}
	return result
}

func ScanInplace(data []int, f func(int, int) int) {
	for idx := 1; idx < len(data); idx++ {
		data[idx] = f(data[idx-1], data[idx])
	}
}

func ReduceSec(data []int, f func(int, int) int) int {
	sum := 0
	for value := range data {
		sum = f(sum, value)
	}
	return sum
}

func ceil(lhs, rhs int) int {
	return (lhs + rhs - 1) / rhs
}
