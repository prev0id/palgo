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
  Дана правильная скобочная последовательность. Нужно найти у каждой скобки соответствующую пару за $O(n) "work"$ и $O(sqrt(n) "polylog" n) "span"$.
]

Напишу идейно алгоритм, писать псевдокод слишком муторно.



= Задание 2
#line-block[
  Дано выражение, где каждая операция обрамлена скобками, а операнды - цифры. Постройте дерево вычислений за $O(n) "work"$ и $O(sqrt(n) "polylog" n) "span"$.
]
