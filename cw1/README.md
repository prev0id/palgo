## Запуск
1) перейти в cw1 директорию

2) собрать (включает локальную установку и сборку библиотеки)
```sh
make build
```

3) запустить
```sh
make run
```

количество потоков настраивается в Makefile, количество запускаемых тестов в main.cpp

## Результаты
```
➜  cw1 git:(master) ✗ make run
PARLAY_NUM_THREADS=4 ./build/main

--- TEST 1 ---
SEQ: 6083.99 ms
PAR: 1946.96 ms
Speedup: 3.12487

--- TEST 2 ---
SEQ: 6060.81 ms
PAR: 2081.93 ms
Speedup: 2.91115

--- TEST 3 ---
SEQ: 6020.87 ms
PAR: 1860.52 ms
Speedup: 3.23612

--- TEST 4 ---
SEQ: 6065.35 ms
PAR: 1784.04 ms
Speedup: 3.39979

--- TEST 5 ---
SEQ: 6083.36 ms
PAR: 2044.17 ms
Speedup: 2.97596

--------------------------
AVERAGE SEQ: 6062.88 ms
AVERAGE PAR: 1943.52 ms
AVERAGE SPEEDUP: 3.11953
--------------------------
```
