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
  Даны m элементов в отсортированном.
  Удалите их из 2-3-дерева за $O(m log n)$ work и $O(log m (log n + log m))$ span.
  (Псевдокод не нужен, опишите просто идею)
]

Пусть $M$ - изначальный массив длины m. $T$ - 2-3-дерево.

+ Получение путей вставки:
  - через `parallel_for` запускаем поиск элементов $M$ в $T$, для каждого $i$-элемента находим лист дерева $p_i$ ему равный.
+ Алгоритм удаления:
  - берем центральный $M[m/2]$ элемент и удаляем его лист $p_(m/2)$
  - делаем перебалансировку, нескольких последующих уровней дерева
  - (пайплайнинг) запускаем удаление на $M[m/4]$ и $M[(3m)/4]$ и так далее внутри `parallel_for`
  - повторяем для каждого (второго?) слоя
  $ "work"=O(m dot log n), "span"=O(log m (log m + log n))  $

= Задание 2

#line-block[
  Постройте 2-3 дерево по отсортированному массиву из n элементов за $O(n)$ work и $O(log n)$ span.
]

```go
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
```
