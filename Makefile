# Actual Binary Name
BIN=pankti

# Build Directory of CMake
CMAKE_BUILD_DIR=build

# CMake built binary
CMAKE_OUTPUT=$(CMAKE_BUILD_DIR)/$(BIN)

# CC
CC:=cc

# TCC Path (currently external kb_text_shaping doesn't support tcc)
TCC=/usr/bin/tcc

# Performance Test Compiler
PERFCC=clang

# Release Mode Compiler
RELEASE_CC=clang

# File to Run on `run` step
FILE=./a.pn

# File to Run for benchmark
PERFFILE=./benchmarks/samples/fib.pn

# The Native Frontend Test Binary
FRONTEND_TEST_BIN=pankti_tests

# Frontend Test Binary Output path for CMake build
FRONTEND_TEST_OUTPUT=$(CMAKE_BUILD_DIR)/$(FRONTEND_TEST_BIN)

# The Native Runtime Test Binary
RUNTIME_TEST_BIN=pankti_runtime_tests
# Runtime Test Binary Output path for CMake build
RUNTIME_TEST_OUTPUT=$(CMAKE_BUILD_DIR)/$(RUNTIME_TEST_BIN)
SAMPLES_DIR=tests/runtime/samples

# Arguments to pass to the testing binary
FRONTEND_TEST_ARGS=
RUNTIME_TEST_ARGS=

KWLOOKUP_OUTPUT:=src/gen/kwlookup.h

CPPCHECK_BUILD_DIR=$(CMAKE_BUILD_DIR)/cppcheck


HEADERS := $(shell find src/ \( -path 'src/gen' -o -path 'src/external' -o -path 'src/tmpl' \) -prune -o -name '*.h' -print)
SOURCES := $(shell find src/ \( -path 'src/gen' -o -path 'src/external' -o -path 'src/tmpl' \) -prune -o -name '*.c' -print)

# =============================================================================
#                          CORE : Primary Actions
# =============================================================================


# Build and Run the Test file
.PHONY: all
all: run

# Build the project with default configuration
.PHONY: build
build:
	cmake --build build --target $(BIN) --parallel

# Run the interpreter with the Test file
.PHONY: run
run: build
	./$(CMAKE_OUTPUT) $(FILE)


# Run code formatter
.PHONY: fmt
fmt:
	@clang-format -i -style=file --verbose $(SOURCES) $(HEADERS)

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


# =============================================================================
#                          TEST(s) : Run Tests
# =============================================================================


.PHONY: _test
_test:
	@cmake --build build --target $(FRONTEND_TEST_BIN) --parallel
	@cmake --build build --target $(RUNTIME_TEST_BIN) --parallel
	@echo "==== Running Frontend Tests ===="
	@./$(FRONTEND_TEST_OUTPUT) $(FRONTEND_TEST_ARGS)
	@echo "==== Finished Frontend Tests ===="
	@echo "==== Running Runtime Tests ===="
	@PANKTI_BIN=$(CMAKE_OUTPUT) SAMPLES_DIR=$(SAMPLES_DIR) ./$(RUNTIME_TEST_OUTPUT)
	@echo "==== Finished Runtime Tests ===="

.PHONY: test
test: build_release _test


.PHONY: retest
retest: _test


.PHONY: test_runtime
test_runtime:
	@cmake --build build --target $(RUNTIME_TEST_BIN)
	@echo "==== Running Runtime Tests ===="
	@PANKTI_BIN=$(CMAKE_OUTPUT) SAMPLES_DIR=$(SAMPLES_DIR) ./$(RUNTIME_TEST_OUTPUT)
	@echo "==== Finished Runtime Tests ===="


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~



# =============================================================================
#                          CONFIGURE(s) : Setup Build Tools
# =============================================================================
.PHONY: configure
configure:
	cmake -S . -B build -DCMAKE_C_COMPILER=$(CC)

.PHONY: configure_tcc
configure_tcc:
	cmake -S . -B build -DCMAKE_C_COMPILER=$(TCC)

.PHONY: configure_clang
configure_clang:
	cmake -S . -B build -DCMAKE_C_COMPILER=clang

.PHONY: configure_ninja
configure_ninja:
	cmake -S . -B build -DCMAKE_C_COMPILER=clang -G "Ninja"

.PHONY: configure_gcc
configure_gcc:
	cmake -S . -B build -DCMAKE_C_COMPILER=gcc


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


# =============================================================================
#                          BUILD(s) : Build The Project
# =============================================================================

.PHONY: build_debug
build_debug:
	rm -rf build
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
	cmake --build build --target $(BIN) --parallel

.PHONY: build_release_ninja
build_release_ninja:
	rm -rf build
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=$(RELEASE_CC) -G "Ninja"
	cmake --build build --target $(BIN) --parallel

.PHONY: build_release
build_release:
	rm -rf build
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=$(RELEASE_CC)
	cmake --build build --target $(BIN) --parallel

.PHONY: build_profile
build_profile:
	rm -rf build
	cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_C_COMPILER=$(RELEASE_CC)
	cmake --build build --target $(BIN) --parallel


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


# =============================================================================
#                          TOOL(s) : Run External Tools
# =============================================================================

.PHONY: run_valgrind
run_valgrind: build
	valgrind --leak-check=full --show-leak-kinds=all -s $(CMAKE_OUTPUT) $(FILE)

.PHONY: run_perf
run_perf: build_profile
	perf record -g --call-graph dwarf -F 999 ./$(CMAKE_OUTPUT) $(PERFFILE)
	perf script -F +pid > pankti.perf

.PHONY: run_callgrind
run_callgrind: build_profile
	valgrind --tool=callgrind --callgrind-out-file="$(BIN).%p.callgrind.out" --dump-instr=yes --simulate-cache=yes $(CMAKE_OUTPUT) $(PERFFILE)

.PHONY: run_clangtidy
run_clangtidy:
	clang-tidy --config-file=.clang-tidy $(SOURCES) $(HEADERS)


.PHONY: run_infer
run_infer: cmake_clean
	infer run --compilation-database build/compile_commands.json

.PHONY: run_cppcheck
run_cppcheck:
	mkdir -p $(CPPCHECK_BUILD_DIR)
	cppcheck --project=pankti.cppcheck


.PHONY: bench
bench: build_release
	bash scripts/run_benchmarks.sh $(CMAKE_OUTPUT)
	pandoc -f markdown -t html -o benchmarks/results/benchmarks.html benchmarks/results/benchmark_results.md


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~



# =============================================================================
#                          Generator(s) : Generate Stuff
# =============================================================================

.PHONY: gen_kwlookup
gen_kwlookup:
	./scripts/kwgperfgen.py src/keywords.h $(KWLOOKUP_OUTPUT)

.PHONY: gen_diagon
gen_diagon:
	./scripts/diagongen.py


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# =============================================================================
#                        Cleaning : Clean Artifcats and Stuff
# =============================================================================

.PHONY: clean_artifacts
clean_artifacts:
	cmake --build build --target clean

.PHONY: clean
clean: clean_artifacts
	rm -rf .cache


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
