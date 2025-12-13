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
  Возьмите алгоритм подсчёта массива ID с лекции и сделайте так, чтобы он использовал $O(n^(1 + epsilon)) quad (0 < epsilon < 1)$ памяти вместо $O(n^2)$,
  сохранив асимптотику work и span, если $epsilon$ - константа.
  Подсказка: время работы изменится где-то в $1/epsilon$ раз.
]

+ Вместо хранения полной таблицы `BB`, на шаге итерации `it` формируем ключи (старый ранг, ранг смещённой позиции и исходный индекс):
  $ K_i = ("ID"["it"][i], "ID"["it"][i+2^"it"] , i), quad i in [0,n) $
+ Отсортировать массив всех ключе по первой и второй компоненте стабильной сортировкой
  - используем поразрядную сортировку в системе счисления с основанием $n^epsilon$
  - реализуем в нескольких `parallel_for`: подсчёт, префикс-сумма, распределение
+ Память:
  - два массива рангов `ID` длины $n$ (текущий и следующий)
  - массив пар `P` длины $n$
  - временный массив для перестановки длины $n$
  - массив счётчиков длины $n^epsilon$
+ После сортировки одинаковые пары будут стоять подряд, и одному блоку подряд идущих элементов можно присвоить один новый ранг $"ID"["it"+1]$
