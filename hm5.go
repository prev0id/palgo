package main

func log(int) int
func ceil(int) int
func sqrt(int) int
func ParallelFor(int, func(int))
func Fork2Join(funcs ...func())
func Reduce(int, func(int, int))
func Scan([]int) []int

// data = {"(", "0", ...}
type Node struct {
	Value string
	Left  *Node
	Right *Node
}

// BuildTree ...
func BuildTree(data string) *Node {
	pairs := FindPairs(data)

	return BuildNode(data, 0, len(pairs), pairs)
}

func BuildNode(data string, l, r int, pairs []int) *Node {
	if r-l == 2 {
		// случай цифры
		return &Node{
			Value: data[l:r],
		}
	}

	opIndex := 0

	if data[l+1] == '(' {
		opIndex = pairs[l+1] + 1
	} else if data[r-1] == ')' {
		opIndex = pairs[r-1] - 1
	} else {
		opIndex = l + 2 // случай (a+b), так как операнды только цифры, то они имеют фиксированную длину 1
	}

	var left, right *Node
	Fork2Join(
		func() { left = BuildNode(data, l+1, opIndex, pairs) },
		func() { right = BuildNode(data, opIndex+1, r-1, pairs) },
	)

	return &Node{
		Value: data[opIndex : opIndex+1],
		Left:  left,
		Right: right,
	}
}

type Pair struct {
	Open  int
	Close int
}

func FindPairs(data string) []int {
	n := len(data)

	deltas := make([]int, n)
	ParallelFor(n, func(idx int) {
		switch data[n] {
		case '(':
			deltas[n] = 1
		case ')':
			deltas[n] = -1
		}
	})

	depths := Scan(delta)

	blocks := ceil(sqrt(n))
	ParallelFor(blocks, func(blockIdx int) {
		// for j :=
	})

}

// func Match(data string) []int {
// 	n := len(data)

// 	matches := make([]int, n)

// 	// пилим все на блоки sqrt
// 	blocks := ceil(sqrt(n))

// 	ParallelFor(blocks, func(i int) {
// 		open, close :=
// 	})

// 	return matches
// }
