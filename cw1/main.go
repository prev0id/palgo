package main

import (
	"log/slog"
	"math/rand"
	"os"
	"slices"
	"strconv"
	"time"
)

const SliceSize = 100_000_000

func main() {
	if len(os.Args) == 1 {
		runWithoutArgs() // обычный запуск на массиве из 10^8 случайных элементов
	} else {
		runWithArgs(os.Args[1:]) // запуск с массивом из аргументов `go run 5 4 3 2 1`
	}
}

func runWithoutArgs() {
	data := make([]int, SliceSize)
	for idx := range SliceSize {
		data[idx] = rand.Int()
	}

	slog.Info("Start Sequential")

	start := time.Now()
	QuickSortSeq(data)
	duration := time.Since(start)

	slog.Info("Sequential", slog.Duration("time", duration))

	if !slices.IsSorted(data) {
		slog.Error("data not sorted")
		os.Exit(1)
	}

	slog.Info("Ok")
}

func runWithArgs(args []string) {
	data := make([]int, len(args))
	for idx := range args {
		value, _ := strconv.Atoi(args[idx])
		data[idx] = value
	}

	QuickSortSeq(data)

	if !slices.IsSorted(data) {
		slog.Error("data not sorted")
		os.Exit(1)
	}

	slog.Info("Ok")
}
