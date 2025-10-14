
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

= Задание 3
#line-block[
  Вам нужно написать функцию $"eps"(a, S)$, которая получает положительных чисел а и возвращает минимальное j,
  такое что сумма элементов $a[1]+...+a[j] >= S$, за $O(j)$ work и $O(log^2 j)$ span.
  Предподсчётом пользоваться нельзя.
]

Основная идея:
+ Ищем суммы черерз `Reduce` на блоках $[2^k-1, 2^(k+1)]$, где $k in [0, floor(log j)]$
$ "work" = 2^(k+1) = O(j) $
$ "span" = sum_(i=0)^(ceil(log j)) log (j / 2^i) = O(log^2 j) $
+ Дальше на последнем отрезке $[2^k-1, 2^(k+1)]$, где $k = floor(log j)$ делаем `Scan` + бинпоиск $j$ по префиксным суммам
$ "work" = O(j) $
$ "span" = O(log(j)) $


```go
func EPS(a []int, S int) int {
	prev_k := 0
	next_k := 1
	sum := 0

	for {
		partial_sum := Reduce(prev_k, next_k, a)
		if partial_sum+sum >= S {
			break
		}
		sum += partial_sum
		prev_k = next_k
		next_k = min(next_k * 2, len(a))
	}

	prefSums := Scan(a[prev_k:next_k])

	prefSumIndex := BinarySearch(
		prefSums, S,
		func(idx int) bool { return prefSums[idx]+sum >= S },
	)

	return prefSumIndex + prev_k
}
```
