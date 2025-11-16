BUILD ?= build
PARLAY_NUM_THREADS ?= 4

.PHONY: build
build:
	mkdir -p $(BUILD)
	cmake -B $(BUILD) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	# cmake -B $(BUILD) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3 -march=native -pipe"
	cmake --build $(BUILD)
	ln -sf $(BUILD)/compile_commands.json compile_commands.json

.PHONY: run
run: build
	PARLAY_NUM_THREADS=$(PARLAY_NUM_THREADS) ./$(BUILD)/main
