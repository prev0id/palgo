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
  Напишите MST на Deterministic Reservations с reserve и commit
]

Моя попытка реализовать парарллельного Краскалу 😬.

Надеюсь с комментами в этот раз будет попроще смотреть код :)

```go
type Edge struct {
	u, v   int
	weight int64
	idx    int // позиция ребра в сортированном массиве ребер (приоритет)
}

func ParallelKruskal(edges []Edge, n int) []Edge {
	// как и в обычном краскале сначала сортируем ребра
	edges = ParallelSort(edges, func(e1, e2 Edge) bool {
		return e1.weight < e2.weight
	})

	// union‑find/disjoint-set реализация будет ниже
	uf := UF{parent: make([]int32, n)}
	// шаги алгоритма с reserve/commit
	steps := make([]Step, n)

	// инициализируем uf и steps
	ParallelFor(0, n, func(idx int) {
		uf.parent[idx] = -1
		steps[idx] = Step{priority: -1}
	})

	mst := make([]Edge, n) // результат
	var mstPos int64 // счётчик записей в mst


	unprocessed := edges
	for len(unprocessed) > 0 {
	 	// delta размер блока который обрабатываем параллельно
		prefix := unprocessed[:delta]

		// Reserve стадия
		ParallelFor(0, delta, func(idx int) {
			edge := prefix[idx]
			// если концы уже в одной компоненте скипаем ребро
			if uf.Find(edge.u) == uf.Find(edge.v) {
				edge.idx = -1
				return
			}
			// пытаемся зарезервировать обе вершины
			// резервируем независимо, в коммите проверим, удалось ли хотя бы одной.
			ok1 := steps[edge.u].reserve(idx)
			ok2 := steps[edge.v].reserve(idx)
			// удалось резервировать, значит годен для коммита
			if ok1 || ok2 {
				edge.idx = idx
			} else { // резервировать не удалось, пропускаем
				edge.idx = -1
			}
		})

		// Commit стадия
		ParallelFor(0, delta, func(i int) {
			edge := prefix[i]
			if edge.idx < 0 {
				return // не зарезервировали
			}
			// проверяем действительно ли зарезервировали данное ребро
			uOk := steps[edge.u].check(idx)
			vOk := steps[edge.v].check(idx)
			// обе вершины перезаписались с более приоритетным ребром
			if !(uOk || vOk) {
				edge.idx = -1
				return
			}
			// записываем ребре в mst, атомарно увеличивая счетчик
			pos := atomic.AddInt64(&mstPos, 1) - 1
			mst[pos] = edge
			// объединяем компоненты в union‑find
			uf.Union(edge.u, edge.v)
			// помечаем ребро как завершённое
			edge.idx = -2
		})

		// сдвигаем неразрешённые ребра в начало массива
		// для каждой позиции i создаём flag=true – ребро не было обработано
		flags := make([]bool, delta)
		ParallelFor(0, delta, func(idx int) {
			flags[idx] = unprocessed[idx].idx == -1
		})
		// получаем индексы позиций в новом массиве
		pos := Scan(flags)
		// обновленный массив новых ребер
		newUnprocessed := make([]Edge, delta)

		ParallelFor(0, delta, func(idx int) {
			if flags[idx] {
				newUnprocessed[pos[idx]] = unprocessed[idx]
			}
			// если ребро уже обработано unprocessed[idx].idx==-2 – просто отбрасываем
		})

		unprocessed = newUnprocessed
		// очищаем резервации
		ParallelFor(0, n, func(idx int) {
			steps[idx].priority = -1
		})
	}
	return mst[:mstPos] // отрезаем лишние нули, если они есть
}
```

Не знаю надо ли было писать реализации step и uf, но вот _по идее_ их потокобезопасные имплементации
```go
type Step struct {
	// храним только приоритет последней (успешной) резервации.
	priority int
}

// pwrite – приоритетная запись записывает v, если v < текущее значение.
func (rs *Step) pwrite(v int) {
	for {
		cur := atomic.LoadInt32(&rs.priority)
		if v >= cur { // уже лучший приоритет записан
			return
		}
		if atomic.CompareAndSwapInt32(&rs.priority, cur, v) {
			return
		}
	}
}

// reserve – попытка зарезервировать вершину с приоритетом p
func (rs *Step) reserve(p int) bool {
	// записываем только если p лучше (меньше) текущего.
	rs.pwrite(p)
	// после записи проверяем, действительно ли p стал новым приоритетом
	cur := atomic.LoadInt32(&rs.priority)
	return rs.check(p)
}

// check – проверка, зарезервировано ли p
func (rs *Step) check(p int) bool {
	return atomic.LoadInt32(&rs.priority) == p
}
```

Реализация системы непересекающихся множеств
```go
type UF struct {
	parent []int
}

// Find с компрессией пути
func (uf *UF) Find(x int) int {
	for {
		p := atomic.LoadInt32(&uf.parent[x])
		if p < 0 {
			return x
		}
		grand := atomic.LoadInt32(&uf.parent[int(p)])
		atomic.CompareAndSwapInt32(&uf.parent[x], p, grand)
		x = p
	}
}

// Union объединяет два множества
func (uf *UF) Union(a, b int) {
	for {
		ra := uf.Find(a)
		rb := uf.Find(b)
		if ra == rb {
			return
		}
		// попытка сделать ra ребёнком rb
		if atomic.CompareAndSwapInt32(&uf.parent[ra], -1, int32(rb)) {
			return
		}
		// eсли не получилось - кто‑то уже изменил структуру, повторяем цикл
	}
}
```
