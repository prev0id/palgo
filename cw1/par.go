package main

import "sync"

const BLOCK = 1000

func Fork2Join(funcs ...func()) {
	wg := &sync.WaitGroup{}

	for _, f := range funcs {
		// f()
		wg.Go(f)
	}

	wg.Wait()
}

func ParallelFor(l, r int, f func(int)) {
	if r-l < BLOCK {
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

func BlockedFor(data []int, f func(int)) {
	ParallelFor(0, ceil(len(data), BLOCK), f)
}

func Scan(data []int, f func(int, int) int) {
	if len(data) < 2*BLOCK {
		ScanInplace(data, f)
		return
	}

	sums := make([]int, ceil(len(data), BLOCK))
	BlockedFor(data, func(idx int) {
		sums[idx] = ReduceSec(data[idx*BLOCK:(idx+1)*BLOCK], f)
	})

	Scan(sums, f)

	BlockedFor(data, func(idx int) {
		data[idx*BLOCK] = f(data[idx*BLOCK], sums[idx])
		ScanInplace(data[idx*BLOCK:(idx+1)*BLOCK], f)
	})
}

func Filter(a []int, f func(int) bool) {

}
