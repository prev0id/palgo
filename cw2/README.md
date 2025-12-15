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
--- TEST 1 ---
SEQ: 10045.8 ms
PAR: 2877.74 ms
SPEEDUP: 3.49085

--- TEST 2 ---
SEQ: 9682.42 ms
PAR: 2838.74 ms
SPEEDUP: 3.41081

--- TEST 3 ---
SEQ: 8983.5 ms
PAR: 2831.04 ms
SPEEDUP: 3.17322

--- TEST 4 ---
SEQ: 8726.68 ms
PAR: 2840.29 ms
SPEEDUP: 3.07246

--- TEST 5 ---
SEQ: 8764.09 ms
PAR: 2876.69 ms
SPEEDUP: 3.04658

--------------------------
AVERAGE SEQ: 9240.49 ms
AVERAGE PAR: 2852.9 ms
AVERAGE SPEEDUP: 3.23898
--------------------------
```
