package main

import (
	"fmt"
)

const BLOCK = 1000

func Reduce(l, r int, a []int, f func(int, int) int) int {
	if abs(l-r) < BLOCK {
		ans := 0
		for i := l; i < r; i++ {
			ans += f(ans, a[i])
		}
		return ans
	}
	m := (l + r) / 2
	var left, right int
	Fork2Join(
		func() { left = Reduce(l, m, a, f) },
		func() { right = Reduce(m, r, a, f) },
	)
	return f(left, right)
}

func Scan(a []int, f func(int, int) int) []int {
	n := len(a)
	sum := make([]int, 2*n-1)
	result := make([]int, n)

	ScanUp(a, sum, 0, 0, n, f)
	ScanDown(a, sum, result, 0, 0, n, 0, f)

	return result
}

func ScanUp(
	a, sum []int,
	node, l, r int,
	f func(int, int) int,
) int {
	if r-l == 1 {
		sum[node] = a[l]
		return sum[node]
	}
	m := (l + r) / 2
	var left, right int
	Fork2Join(
		func() { left = ScanUp(a, sum, 2*node+1, l, m, f) },
		func() { right = ScanUp(a, sum, 2*node+2, m, r, f) },
	)
	sum[node] = f(left, right)
	return sum[node]
}

func ScanDown(
	a, sum,
	res []int,
	node, l, r, fromLeft int,
	f func(int, int) int,
) {
	if r-l == 1 {
		res[l] = fromLeft + a[l]
		return
	}

	m := (l + r) / 2

	leftSum := sum[2*node+1]

	Fork2Join(
		func() { ScanDown(a, sum, res, 2*node+1, l, m, fromLeft, f) },
		func() { ScanDown(a, sum, res, 2*node+2, m, r, fromLeft+leftSum, f) },
	)
}

// func EPS(a []int, S int) int {
// 	prev_k := 0
// 	next_k := 1
// 	sum := 0

// 	for {
// 		partial_sum := Reduce(prev_k, next_k, a)
// 		if partial_sum+sum >= S {
// 			break
// 		}
// 		sum += partial_sum
// 		prev_k = next_k + 1
// 		next_k = next_k * 2
// 	}

// 	prefSums := Scan(a[prev_k:next_k])

// 	prefSumIndex := BinarySearch(
// 		prefSums, S,
// 		func(idx int) bool { return prefSums[idx]+sum >= S },
// 	)

// 	return prefSumIndex + prev_k
// }

func abs(value int) int {
	if value < 0 {
		return value
	}
	return value
}

func main() {
	result := Scan(
		[]int{6, 4, 16, 10, 16, 14, 2, 8},
		func(i1, i2 int) int { return i1 + i2 },
	)
	fmt.Println(result)
}
