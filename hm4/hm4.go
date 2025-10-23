package main

type Node struct {
	id int
}

type Edge struct {
	From, To int
}

func Depth(n int, r int, traverse []Edge) []int {
	// получаем родителей для каждой вершины разрывом цикла после второго вхождения r
	// и подсчетом List ranking
	parents := GetParents(n, r, traverse)

	weights := make([]int, len(traverse))
	ParallelFor(len(traverse), func(idx int) {
		edge := traverse[idx]
		weight := -1 // для ребер сын->родитель
		if edge.From == parents[edge.To] {
			weight = 1 // для ребер родитель->сын
		}
		weights[idx] = weight
	})

	// получаем глубину вершин в которые приходим на каждом шаге обхода
	depths := Scan(weights)

	result := make([]int, n)
	ParallelFor(len(traverse), func(idx int) {
		edge := traverse[idx]
		if edge.From == parents[edge.To] {
			result[edge.To] = depths[idx]
		}
	})

	return result
}
```

func GetParents(n int, r int, traverse []Edge) []int

func Sort(n int, A []int) []int {
	histograms := make([][]int, n/log(n))

	blockLen := log(n)
	blocks := ceil(n / blockLen)

	// делаем линейный подсчет на блоках длины len(n)
	ParallelFor(blocks, func(idx int) {
		blockStart := idx * blockLen
		blockEnd := min((idx+1)*blockLen, n)

		histograms[idx] = calcHistogram(blockLen, A[blockStart:blockEnd])
	})

	// собираем все посчеты в единную гистограму
	Reduce(blocks, func(block1 int, block2 int) {
		for i := range blockLen {
			histograms[block1][i] += histograms[block2][i]
		}
	})

	histogram := histograms[0]

	// индексы блоков с одинаковыми элементами
	postitions := Scan(histogram)

	result := make([]int, n)

	ParallelFor(blocks, func(element int) {
		start := postitions[element]
		end := postitions[element+1] // or n

		ParallelFor(end-start, func(idx int) {
			result[idx] = element
		})
	})

	return result
}

func calcHistogram(length int, A []int) []int {
	histogram := make([]int, length)
	for _, el := range A {
		histogram[el] += 1
	}
	return histogram
}

func log(int) int
func ceil(int) int
func ParallelFor(int, func(int))
func Reduce(int, func(int, int))
func Scan([]int) []int
