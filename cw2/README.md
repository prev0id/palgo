## Запуск
1) перейти в cw2 директорию

2) собрать (включает локальную установку и сборку библиотеки)
```sh
make build
```

3) запустить
```sh
make run
```

## Результат

```
PARLAY_NUM_THREADS=4 ./build/main
ALL TESTS PASSED

--- TEST 1 ---
SEQ: 8201.09 ms
PAR: 2789.55 ms
Speedup: 2.93993

--- TEST 2 ---
SEQ: 8680.25 ms
PAR: 2808.42 ms
Speedup: 3.0908

--- TEST 3 ---
SEQ: 8192.84 ms
PAR: 2818.42 ms
Speedup: 2.90689

--- TEST 4 ---
SEQ: 9203.95 ms
PAR: 2812.3 ms
Speedup: 3.27275

--- TEST 5 ---
SEQ: 8439.46 ms
PAR: 2800.38 ms
Speedup: 3.01368

--------------------------
AVERAGE SEQ: 8543.52 ms
AVERAGE PAR: 2805.82 ms
AVERAGE SPEEDUP: 3.04493
--------------------------
```
