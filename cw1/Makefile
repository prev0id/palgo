PARLAY?=$(PWD)/deps/parlaylib
BUILD?=$(PARLAY)/build
INCLUDE?=$(PWD)/include
BIN?=$(PWD)/bin
PARLAY_NUM_THREADS?=4

.PHONY: build
build: clear-parlaylib prepare-parlaylib build-parlaylib build-app

.PHONY: clear-parlaylib
clear-parlaylib:
	rm -rf $(BUILD) || true
	rm -rf $(INCLUDE) || true

.PHONY: prepare-parlaylib
prepare-parlaylib:
	mkdir $(BUILD)
	mkdir $(INCLUDE)

.PHONY: build-parlaylib
build-parlaylib:

	cd $(BUILD) \
	&& cmake $(PARLAY) -DCMAKE_INSTALL_PREFIX:PATH=$(INCLUDE) \
	&& cmake --build . --target install

.PHONY: build-app
build-app:
	g++ -std=c++17 main.cpp -o $(BIN)/main

.PHONY: run
run:
	PARLAY_NUM_THREADS=$(PARLAY_NUM_THREADS) $(BIN)/main
