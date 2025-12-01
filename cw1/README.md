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
CPU: Apple M4 Pro

OS: MacOS 15.6.1

```
➜  cw1 git:(master) ✗ make run
PARLAY_NUM_THREADS=4 ./build/main

--- TEST 1 ---
SEQ: 6111.07 ms
PAR: 1863.69 ms
Speedup: 3.27901

--- TEST 2 ---
SEQ: 6264.51 ms
PAR: 1878.1 ms
Speedup: 3.33555

--- TEST 3 ---
SEQ: 6196.45 ms
PAR: 1743.48 ms
Speedup: 3.55408

--- TEST 4 ---
SEQ: 6173.03 ms
PAR: 1662.06 ms
Speedup: 3.71408

--- TEST 5 ---
SEQ: 6395.6 ms
PAR: 1868.13 ms
Speedup: 3.42353

--------------------------
AVERAGE SEQ: 6228.13 ms
AVERAGE PAR: 1803.09 ms
AVERAGE SPEEDUP: 3.45414
--------------------------
```
