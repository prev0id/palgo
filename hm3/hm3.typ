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
Пусть $M$ - изначальный массив длины m. $T$ 2-3-дерево.

+ Параллельно разбиваем дерево на $m+1$ поддеревьев ($T_1, ..., T_(m+1)$), которые содержат $M$ элементы на границах.
  Разбиваем M пополам, ищем центральный элемент в дереве $O(log n)$, помечаем путь до него
  Вызываем рекурентно алгоритм с помощью `Fork2Join` на левой и правой половинке M, глубина рекурсии $O(log m)$
  $ "work"=m dot log n, "span"=log m log n $
+ Параллельно выкидываем помеченные элементы из поддеревьев $T_i$
  $ "work"=m, "span"=log m $
+ Мерджим поддеревья обратно. Параллельно мерджим пары $(T_1,T_2)$, $(T_3,T_4)$, etc. потом $(T_12,T_34)$ и тд
  Количество мерджий: $O(log m)$, стоимость мерджа $O(log m + log n)$ (Деревья будут отличаться по крайам)
  $ "work"=m dot log n, "span"=log m (log m + log n) $


= Задание 2
#line-block[
  Постройте 2-3 дерево по отсортированному массиву из n элементов за $O(n)$ work и $O(log n)$ span.
]

```go
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
```

= Задание 3\*
#line-block[
  Придумайте такие асимптотики для алгоритмов, чтобы имело смысл использовать 3 уровня accelerating cascades.
  + R, который решает задачу за $O("RW"(n))$ work и $O("RS"(n))$ span.
  + Q, который уменьшает задачу в $"QX"=O(1)$ раз, за $O("QW"(n))$ work и $O("QS"(n))$ span.
  + R, который решает задачу за $O("RW"(n))$ work и $O("RS"(n))$ span.
]

Я пас
