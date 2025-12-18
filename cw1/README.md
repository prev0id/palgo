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
--- TEST 1 ---
SEQ: 6335.27 ms
PAR: 1900.67 ms
SPEEDUP: 3.33317

--- TEST 2 ---
SEQ: 6371.35 ms
PAR: 1846.93 ms
SPEEDUP: 3.44969

--- TEST 3 ---
SEQ: 6352.99 ms
PAR: 1780.3 ms
SPEEDUP: 3.5685

--- TEST 4 ---
SEQ: 6414.81 ms
PAR: 1909.7 ms
SPEEDUP: 3.35906

--- TEST 5 ---
SEQ: 6531.97 ms
PAR: 1927.43 ms
SPEEDUP: 3.38896

--------------------------
AVERAGE SEQ: 6401.28 ms
AVERAGE PAR: 1873.01 ms
AVERAGE SPEEDUP: 3.41765
--------------------------
```
