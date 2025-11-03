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
  Постройте пример графа, на котором алгоритм без прицепления звёзд (т.е., без второй стадии) будет работать за линейный span.
]

В голову приходит только подобный граф из соединенных звезд, которые не могут быть соединены в первой стадии.
Проблема в том что не очевидно почему на данном графе алгоритм вообще должен сходиться, так как прыжки из третьей стадии никак на высоте деревьев не сказываются.
Поэтому сколько итераций не делай, получаем одну и туже картину.

P.S. скорее всего не понял до конца как алгоритм должен работать :)

#figure(
  image("hm6_idk.png", width: 80%)
)
