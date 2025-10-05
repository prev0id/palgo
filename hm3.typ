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

= Задание 2
#line-block[
  Постройте 2-3 дерево по отсортированному массиву из n элементов за $O(n)$ work и $O(log n)$ span.
]

= Задание 3\*
#line-block[
  Придумайте такие асимптотики для алгоритмов, чтобы имело смысл использовать 3 уровня accelerating cascades.
  + R, который решает задачу за $O("RW"(n))$ work и $O("RS"(n))$ span.
  + Q, который уменьшает задачу в $"QX"=O(1)$ раз, за $O("QW"(n))$ work и $O("QS"(n))$ span.
  + R, который решает задачу за $O("RW"(n))$ work и $O("RS"(n))$ span.
]
