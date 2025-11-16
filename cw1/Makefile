BUILD ?= build
PARLAY_NUM_THREADS ?= 4

.PHONY: build
build:
	mkdir -p $(BUILD)
	cmake -B $(BUILD) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	cmake --build $(BUILD)
	ln -sf $(BUILD)/compile_commands.json compile_commands.json

.PHONY: run
run:
	PARLAY_NUM_THREADS=$(PARLAY_NUM_THREADS) ./$(BUILD)/main
