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
  Предъявите граф с РОВНО $m <= n^(2- epsilon)$ рёбер ($m$ задано, но меньше $n^(2-epsilon)$,
  где $epsilon$ любая положительная константа), на котором рандомизированный алгоритм поиска компонент сильной связности работает
  за $Omega(m log n)$ в матожидании. Не забудьте про доказательство.
]

#figure(
  image("graph.png", width: 70%)
)

Сделаем граф данного вида, цепочка из примерно n вершин, с двудольным графом $K_(p,p)$ на конце $p ~ sqrt(m) = n^(1-epsilon)$.

Выбираемая в алгоритме вершина будет чаще всего в цепочке, а не на $K_(p,p)$.
Рекурсия в серднем должна занять $log n$ раз, в среднем разделяю длину цепочки в 2 раза,
каждый рекурсивный вызов который не попал на $K_(p,p)$, будет обрабатывать весь двудольный граф, со всеми его $~m$ ребрами.


= Задание 2
#line-block[
  Докажите, что пирамидка построенная на координатах $(D, 0, ...), (0, D, 0, ...), (0, 0, D, 0, ...)$ в n-мерном пространстве имеет объём $D^n/n!$.
]

+ База:
  $ n = 2: quad V_2 = D^2/2 $
+ Пусть верно для произвольного $n >= 2: thick V_n = D^n/n!$.

  Рассмотрим сечение $s$, параллелльное $(x_1, ..., x_n)$, пересекающее ось $x_(n+1)$ в точке $S$. В силу подобия, объем тетраедара в сечении будет равен $((D-S)/D)^n dot V_n$,
  Тогда объем $n+1$-мерной пирамиды, вычисляется как
  $ integral_0^D V_n ((D-s)/D)^n dif s
   = -V_n dot D/(n+1) dot lr(((D-s)/D)^(n+1)|)_0^D
   = V_n dot D/(n+1) = D^(n+1)/(n+1)! $
+ По индукции, следует $V_n = D^n/n!$
  #figure(
    image("desmos-graph.png", width: 70%)
  )
