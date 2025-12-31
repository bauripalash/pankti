# Actual Binary Name
BIN=pankti

# Build Directory of CMake
CMAKE_BUILD_DIR=build

# Zig Out Directory
ZIG_BUILD_DIR=zig-out

# CMake built binary
CMAKE_OUTPUT=$(CMAKE_BUILD_DIR)/$(BIN)

# Zig built binary
ZIG_OUTPUT=$(ZIG_BUILD_DIR)/bin/$(BIN)

# CC
CC=cc

# TCC Path (for future use)
TCC=/usr/bin/tcc

# Performance Test Compiler
PERFCC=clang

# Release Mode Compiler
RELEASE_CC=clang

# File to Run on `run` step
FILE=./a.pank

# File to Run for benchmark
PERFFILE=./benchmarks/fib.pank

# The Native Frontend Test Binary
FRONTEND_TEST_BIN=pankti_tests

# Frontend Test Binary Output path for CMake build
FRONTEND_TEST_OUTPUT=$(CMAKE_BUILD_DIR)/$(FRONTEND_TEST_BIN)

# The Native Runtime Test Binary
RUNTIME_TEST_BIN=pankti_runtime_tests
# Runtime Test Binary Output path for CMake build
RUNTIME_TEST_OUTPUT=$(CMAKE_BUILD_DIR)/$(RUNTIME_TEST_BIN)
SAMPLES_DIR=tests/runtime/samples

# Test Binary Output path for Zig build
ZIG_TEST_OUTPUT=$(ZIG_BUILD_DIR)/bin/$(FRONTEND_TEST_BIN)

# Arguments to pass to the testing binary
FRONTEND_TEST_ARGS=
RUNTIME_TEST_ARGS=


HEADERS:= $(shell find src/include -path 'src/external' -prune -o -path 'src/include/exported' -prune -o -name '*.h' -print)
SOURCES:= $(shell find src/ -path 'src/external' -prune -o -name '*.c' -print)


all: run
.PHONY: crun
crun:
	make run COMPILER=true

.PHONY: build
build:
	cmake --build build --target $(BIN)

.PHONY: run
run: build
	./$(CMAKE_OUTPUT) $(FILE)

.PHONY: test
test: build_rls
	@cmake --build build --target $(FRONTEND_TEST_BIN)
	@cmake --build build --target $(RUNTIME_TEST_BIN)
	@echo "==== Running Frontend Tests ===="
	@./$(FRONTEND_TEST_OUTPUT) $(FRONTEND_TEST_ARGS)
	@echo "==== Finished Frontend Tests ===="
	@echo "==== Running Runtime Tests ===="
	@PANKTI_BIN=${CMAKE_OUTPUT} SAMPLES_DIR=${SAMPLES_DIR} ./${RUNTIME_TEST_OUTPUT}
	@echo "==== Finished Runtime Tests ===="


.PHONY: zbuild
zbuild:
	zig build

.PHONY: zrun
zrun: zbuild
	./$(ZIG_OUTPUT) $(FILE)

.PHONY: ztest
ztest:
	@zig build ntests -Doptimize=ReleaseFast
	@echo "==== Running Frontend Tests ===="
	@./$(ZIG_TEST_OUTPUT) $(FRONTEND_TEST_ARGS)
	@echo "==== Finished Frontend Tests ===="
	@echo "==== Running Runtime Tests ===="
	@zig build -Doptimize=ReleaseFast
	@PANKTI_BIN=$(ZIG_OUTPUT) python -m unittest discover -s tests --verbose
	@echo "==== Finished Runtime Tests ===="

.PHONY: runtime_tests
runtime_tests:
	@cmake --build build --target $(RUNTIME_TEST_BIN)
	@echo "==== Running Runtime Tests ===="
	@PANKTI_BIN=${CMAKE_OUTPUT} SAMPLES_DIR=${SAMPLES_DIR} ./${RUNTIME_TEST_OUTPUT}
	@echo "==== Finished Runtime Tests ===="

.PHONY: fmt
fmt:
	@clang-format -i -style=file --verbose $(SOURCES) $(HEADERS)

.PHONY: valgrind
valgrind: build
	valgrind --leak-check=full --show-leak-kinds=all -s $(CMAKE_OUTPUT)

.PHONY: cmake_tcc
cmake_tcc:
	cmake -S . -B build -DCMAKE_C_COMPILER=$(TCC)

.PHONY: cmake_setup
cmake_setup:
	cmake -S . -B build -DCMAKE_C_COMPILER=clang

.PHONY: cmake_clang
cmake_cc:
	cmake -S . -B build -DCMAKE_C_COMPILER=clang

.PHONY: cmake_clean
cmake_clean:
	cd build && make clean

.PHONY: build_dbg
build_dbg:
	rm -rf build
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
	cmake --build build --target $(BIN)

.PHONY: build_rls
build_rls:
	rm -rf build
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=$(RELEASE_CC)
	cmake --build build --target $(BIN)

.PHONY: build_rld
build_rld:
	rm -rf build
	cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_C_COMPILER=$(RELEASE_CC)
	cmake --build build --target $(BIN)

.PHONY: run_perf
run_perf: build_rld
	perf record -g --call-graph dwarf -F 999 ./$(CMAKE_OUTPUT) $(PERFFILE)
	perf script -F +pid > pankti.perf

.PHONY: run_callgrind
run_callgrind: build_rld
	valgrind --tool=callgrind --callgrind-out-file="$(BIN).%p.callgrind.out" --dump-instr=yes --simulate-cache=yes $(CMAKE_OUTPUT) $(PERFFILE)

.PHONY: run_clangtidy
run_clangtidy:
	clang-tidy --config-file=.clang-tidy $(SOURCES) $(HEADERS)

.PHONY: run_benchmarks
run_benchmarks: build_rls
	bash run_benchmarks.sh $(CMAKE_OUTPUT)
	pandoc -f markdown -t html -o benchmarks.html benchmark_results.md

.PHONY: infer
infer: cmake_clean
	infer run --compilation-database build/compile_commands.json

.PHONY: clean
clean: cmake_clean
	rm -rf .zig-cache
	rm -rf .cache
	rm -rf zig-out


