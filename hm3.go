package main

import (
	"sync"
)

func Fork2Join(funcs ...func()) {
	wg := &sync.WaitGroup{}

	for _, f := range funcs {
		// f()
		wg.Go(f)
	}

	wg.Wait()
}

func ParallelFor(l, r int, f func(int)) {
	if abs(l-r) < BLOCK {
		for i := l; i < r; i++ {
			f(i)
		}
		return
	}
	m := (l + r) / 2
	Fork2Join(
		func() { ParallelFor(l, m, f) },
		func() { ParallelFor(m, r, f) },
	)
}

type Node struct {
	Key      int
	Children []*Node
}

func Build23Tree(data []int) *Node {
	return buildNode(data, 0, len(data))
}

func buildNode(data []int, l, r int) *Node {
	n := r - l
	if n == 1 {
		return &Node{Key: data[l]}
	}

	var children []*Node

	if n%3 == 1 || n == 2 {
		var left, right *Node
		m := (l + r) / 2
		Fork2Join(
			func() { left = buildNode(data, l, m) },
			func() { right = buildNode(data, l, m) },
		)
		children = []*Node{left, right}
	} else {
		var left, middle, right *Node
		m1 := l + n/3
		m2 := l + 2*n/3
		Fork2Join(
			func() { left = buildNode(data, l, m1) },
			func() { middle = buildNode(data, m1, m2) },
			func() { right = buildNode(data, m2, r) },
		)
		children = []*Node{left, middle, right}
	}

	return &Node{Children: children}
}
