# Actual Binary Name
BIN ?= pankti

# Build Directory of CMake
CMAKE_BUILD_DIR ?= build

# Zig Out Directory
ZIG_BUILD_DIR ?= zig-out

# CMake built binary
CMAKE_OUTPUT ?= $(CMAKE_BUILD_DIR)/$(BIN)

# Zig built binary
ZIG_OUTPUT ?= $(ZIG_BUILD_DIR)/bin/$(BIN)

# CC
CC:=

# TCC Path (for future use)
TCC ?= /usr/bin/tcc

# Performance Test Compiler
PERFCC ?=clang

# Release Mode Compiler
RELEASE_CC ?= clang

# File to Run on `run` step
FILE ?= ./a.pank

# File to Run for benchmark
PERFFILE ?= ./fib35.pank

# The Native Test Binary
TEST_BIN ?= pankti_tests

# Test Binary Output path for CMake build
TEST_OUTPUT ?= $(CMAKE_BUILD_DIR)/tests/$(TEST_BIN)

# Test Binary Output path for Zig build
ZIG_TEST_OUTPUT ?= $(ZIG_BUILD_DIR)/bin/$(TEST_BIN)

# Arguments to pass to the testing binary
TEST_ARGS ?= 


HEADERS:= $(shell find src/include -path 'src/external' -prune -o -path 'src/include/exported' -prune -o -name '*.h' -print)
SOURCES:= $(shell find src/ -path 'src/external' -prune -o -name '*.c' -print)


all: run

.PHONY: build
build:
	cmake --build build --target $(BIN)

.PHONY: run
run: build
	./$(CMAKE_OUTPUT) $(FILE)

.PHONY: test
test:
	cmake --build build --target $(TEST_BIN)
	./$(TEST_OUTPUT) $(TEST_ARGS)


.PHONY: zbuild
zbuild:
	zig build

.PHONY: zrun
zrun: zbuild
	./$(ZIG_OUTPUT) $(FILE)

.PHONY: ztest
ztest:
	zig build ntests
	./$(ZIG_TEST_OUTPUT) $(TEST_ARGS)

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
	cmake -S . -B build -DCMAKE_C_COMPILER=$(CC)

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

.PHONY: run_perf
run_perf:
	rm -rf build
	cmake -S . -B build -DCMAKE_C_COMPILER=$(PERFCC) -DCMAKE_BUILD_TYPE=Release
	cmake --build build --target $(BIN)
	perf record -g --call-graph dwarf -F 999 ./$(CMAKE_OUTPUT) $(PERFFILE)
	perf script -F +pid > pankti.perf


.PHONY: infer
infer: cmake_clean
	infer run --compilation-database build/compile_commands.json

.PHONY: clean
clean: cmake_clean
	rm -rf .zig-cache
	rm -rf .cache
	rm -rf zig-out


