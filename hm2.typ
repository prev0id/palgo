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
  Доказать, что $ sum_(i=0)^(log n) log(n / 2^i) = Theta(log^2 n) $
]

$ sum_(i=0)^(log n)log(n / 2^i) = sum_(i=0)^(log n) log(n) - i = (log(n) + 1) dot log(n) - sum_(i=0)^(log n) i = $
$ = (log(n) + 1) dot log(n) - log(n) dot (log(n) + 1) / 2 $
$ = ((log(n) + 1) dot log(n) ) / 2 = 1/2 log^2(n) + 1/2 log(n) $

Для $n>4$ верно:
$ 1/2 log^2(n) <= 1/2 log^2(n) + 1/2 log(n) <= 2 log^2(n) $
$ 1/2 log^2(n) + 1/2 log(n) in Theta(log^2(n)) $

= Задание 2
#line-block[
  Написать код алгоритма scan в fork-join модели с $O(n)$ work и $O(log n)$ span.
]
```go
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
```


= Задание 3
#line-block[
  Вам нужно написать функцию $"eps"(a, S)$, которая получает положительных чисел а и возвращает минимальное j,
  такое что сумма элементов $a[1]+...+a[j] >= S$, за $O(j)$ work и $O(log^2 j)$ span.
  Предподсчётом пользоваться нельзя.
]

```go
func EPS(a []int, S int) int {
	l := 0
	r := len(a)
	result := r

	for l <= r {
		m := (l + r) / 2
		sum := Reduce(0, m, a)

		if sum > S {
			result = m
			r = m
		} else {
			l = m + 1
		}
	}
	return result
}
```

= Задание 4
#line-block[
  Опишите алгоритм нахождения всех простых до $N$ за $O(N log log N)$ work и $O("polylog" N)$ span.
  (Желательно за $O(log N dot log log N)$ span). Тут можно пользоваться parallel_for, map, scan и filter.
  Псевдокод даже необязательно.
]
Идея в том чтобы вычислять решето Эратосфена и с помощью `parallel_for` вычеркивать кратные числа,
для этого создается булевый массив в котором помечаются числа которые не могут быть простыми:

+ Если `N==2` возращаем 2
+ Ищем рекурсивно простые до $sqrt(N)$
+ Вычеркиваем с `parallel_for` все простые от $sqrt(N)$ до $N$
+ Фильтруем и возвращаем простые

- Span
$ "глубина рекурсии" N^(1/"Depth") = 2 => "Depth" = log log N $
$ "пометка составных и фильтрация простых требует" O(log N) => "span"=O(log N dot log log N) $

- Work
$ "work"=O(N log log N) "- обычная сложность решета эратосфена" $
