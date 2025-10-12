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
	leaves := make([]*Node, len(data))
	ParallelFor(
		0, len(data),
		func(i int) { leaves[i] = &Node{Key: data[i]} },
	)

	level := buildLevel(leaves)

	return level[0]
}

func buildLevel(nodes []*Node) []*Node {
	if len(nodes) == 1 { // корень
		return nodes
	}

	n := len(nodes)
	levelSize := (n + 2) / 3
	level := make([]*Node, levelSize)

	ParallelFor(0, levelSize, func(i int) {
		start := i * 3
		end := min(start+3, n)
		level[i] = &Node{Children: nodes[start:end]}
	})

	return buildLevel(level)
}
