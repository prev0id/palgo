package main

// https://neerc.ifmo.ru/wiki/index.php?title=Быстрая_сортировка
func QuickSortSeq(data []int) {
	if len(data) <= 1 {
		return
	}
	m := partitionSeq(data)
	QuickSortSeq(data[:m+1])
	QuickSortSeq(data[m+1:])
}

func partitionSeq(data []int) int {
	l, r := 0, len(data)-1

	pivot := data[(l+r)/2]

	for l <= r {
		for data[l] < pivot {
			l++
		}
		for data[r] > pivot {
			r--
		}
		if l >= r {
			break
		}
		data[l], data[r] = data[r], data[l]
		l++
		r--
	}
	return r
}
