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

Прошу прощение за отсутствие кода, нет времени чтобы это нормально закодить

= Задание 1
#line-block[
  Дана правильная скобочная последовательность. Нужно найти у каждой скобки соответствующую пару за $O(n) "work"$ и $O(sqrt(n) "polylog" n) "span"$.
]

+ Разбиваем строку на $sqrt(n)$ блоков
  - Паралелльно на каждом блоке $k$ вычисляем пары, внутри блока алгоритм последовательный
  - Сохраняем индексы скобочек без пар в массивы $"open"[k]$, $"close"[k]$
  $ "work" = O(n), "scan" = O(sqrt(n) dot log sqrt(n)) = O(sqrt(n) dot log n) $
+ Дальше строим бинарное дерево
  - В листях храним указатели на массивы $"open"[k]$, $"close"[k]$
  - В вершинах храним количество незаматченных открывющихся и закрывающихся скобок $("len"("open"), "len"("close"))$,
    а так же число пар, которые образуются $"match" = min("len"("left_child.open"),"len"("right_child.close"))$
  $ "work" = O(n), "scan" = O(log sqrt(n)) = O(log n) $
+ Получается по дереву из п.2 мы можем за $log sqrt(n)$ найти пару для каждой оставшейся скобки
  - Для $"open"[i][j]$ (j-я открывающаяся скобка из i-го блока).
      - Поднимаемся по дереву вверх, пока сумма $"match"$ пройденных вершин не привысит $"len"("open"[i]) - j$
      - Cпускаемся по правому поддереву найденной вершины, ищя закрывающуюся скобку с нужным индексом, мы можем это сделать так как каждая вершина хранит количество скобок без пар
  - в худшем случае поиск n скобок, за $log sqrt(n)$ каждая в `ParallelFor`, займет:
  $ "work" = O(n), "scan" = O(log n dot log sqrt(n)) = O(log^2 n) $


= Задание 2
#line-block[
  Дано выражение, где каждая операция обрамлена скобками, а операнды - цифры. Постройте дерево вычислений за $O(n) "work"$ и $O(sqrt(n) "polylog" n) "span"$.
]

+ Разбиваем строку на $sqrt(n)$ блоков
  - Паралелльно на каждом блоке $k$ вычисляем пары, внутри блока алгоритм последовательный
  - Сохраняем индексы скобочек без пар в массивы $"open"[k]$, $"close"[k]$
  $ "work" = O(n), "scan" = O(sqrt(n) dot log sqrt(n)) = O(sqrt(n) dot log n) $
+
