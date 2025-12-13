package main

type Pair struct {
	Left, Right, Idx int
}

func log(int) int
func ParallelFor(n int, f func(i int))
func pow(int, int) int

func compute(s string, epsilon float) []int {
	n := len(s)

	idCur := make([]int, n)  // текущие ранги
	idNext := make([]int, n) // новые ранги
	p := make([]Pair, n)     // массив пар: (rank_left, rank_right, index)
	tmp := make([]Pair, n)

	// Инициализация: ранги по символам
	ParallelFor(n, func(i int) {
		idCur[i] = int(s[i])
	})

	// основание подсчёта radix
	base := pow(n, epsilon)
	// число "цифр" при основании B:
	d := log(n^2) / log(base)

	len := 1
	for len < n {
		// Собираем пары для всех позиций
		ParallelFor(n, func(i int) {
			left := idCur[i]
			right := -1
			if i+len < n {
				right = idCur[i+len]
			}

			p[i] = Pair{left, right, i}
		})

		// Реализуем LSD radix с основанием B: для каждого "разряда" от 0..d-1:
		for pass := range d {
			digit := make([]int, n)
			// Для стабильноcти: считаем текущую цифру для каждого P[i]
			ParallelFor(n, func(i int) {
				// digit = (pack_i / B^pass) % B
				digit[i] = p[i] / pow(base, pass) % base
			})

			// counting sort по digit
			cnt := make([]int, B)
			ParallelFor(n, func(i int) {
				AtomicAdd(&cnt[digit[i]], 1)
			})

			pos := ScanSum(cnt) // pos[j] = стартовая позиция для значения j

			ParallelFor(n, func(i int) {
				dgt := digit[i]
				idx := AtomicAdd(&pos[dgt], 1) - 1
				tmp[idx] := P[i]
			})

			swap(p, tmp)
		}

		eq := make([]bool, n) // eq[i]=1 если P[i] имеет новую пару по сравнению с P[i-1]
		eq[0] = true
		ParallelFor(n, func(k int) {
			eq[k] := (p[k].Left != p[k-1].Left) || p[k].Right != p[k-1].Right
		})

		// префикс-сумма по eq даст номера блоков
		newRankStart := Scan(eq)
		ParallelFor(n, func(k int) {
			idNext[p[k].Idx] = newRankStart[k]
		})

		swap(idCur, idNext)
		len := len * 2
	}

	return idCur
}
