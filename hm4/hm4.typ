#set page(
  paper: "a4",
  numbering: "1",
  number-align: right,
  header: [
    #smallcaps[Деев Семен Алексеевич]
    #h(1fr) ВШЭ - БКНАД222
  ],
)

#set par(justify: true)

#show raw.where(block: true): code => {
  show raw.line: line => {
    text(fill: gray)[#line.number]
    h(1em)
    line.body
  }
  code
}

#show raw.where(block: true): block.with(
  fill: luma(250),
  inset: 10pt,
  radius: 4pt,
)

#show link: set text(fill: blue, weight: 700)
#show link: underline

#let line-block = rect.with(fill: luma(240), stroke: (left: 0.25em))

= Задание 1
#line-block[
  Отсортировать $n$ элементов не больше $log n$ за $O(n) "work"$ и $O("polylog" n) "span"$ (без конкурентных операций).
]

```go
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
		end := postitions[element+1]

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
```


= Задание 2
#line-block[
  Дано подвешенное дерево за вершину $r$ с уже построенным Эйлеровым обходом.
  Нужно для каждой вершины найти её глубину в дереве за $O(n) "work"$ и $O("polylog" n) "span"$.
]

```go
type Edge struct {
	From, To int
}

func Depth(n int, r int, traverse []Edge) []int {
	// получаем родителей для каждой вершины разрывом цикла после последнего вхождения r и подсчетом List ranking
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
