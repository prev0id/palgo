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


= Задание 2
#line-block[
  Дано подвешенное дерево за вершину $r$ с уже построенным Эйлеровым обходом.
  Нужно для каждой вершины найти её глубину в дереве за $O(n) "work"$ и $O("polylog" n) "span"$.
]
