BIN:=pankti
CMAKE_OUTPUT:=build/$(BIN)
ZIG_OUTPUT:=zig-out/bin/$(BIN)
TCC:=/usr/bin/tcc
PERFCC:=clang

HEADERS:= $(shell find src/include -path 'src/external' -prune -o -path 'src/include/exported' -prune -o -name '*.h' -print)
SOURCES:= $(shell find src/ -path 'src/external' -prune -o -name '*.c' -print)


all: run

.PHONY: build
build:
	cmake --build build

.PHONY: run
run: build
	./$(CMAKE_OUTPUT)

.PHONY: fmt
fmt:
	@clang-format -i -style=file --verbose $(SOURCES) $(HEADERS)

.PHONY: valgrind
valgrind: build
	valgrind --leak-check=full --show-leak-kinds=all -s $(CMAKE_OUTPUT)

.PHONY: cmake_tcc
cmake_tcc:
	cmake -S . -B build -G Ninja -DCMAKE_C_COMPILER=$(TCC)

.PHONY: cmake_setup
cmake_setup:
	cmake -S . -B build

.PHONY: cmake_clang
cmake_clang:
	cmake -S . -B build -DCMAKE_C_COMPILER=clang

.PHONY: cmake_clean
cmake_clean:
	cd build

.PHONY: run_perf
run_perf:
	rm -rf build
	cmake -S . -B build -DCMAKE_C_COMPILER=$(PERFCC) -DCMAKE_BUILD_TYPE=Release
	cmake --build build
	perf record -g --call-graph dwarf -F 999 ./$(CMAKE_OUTPUT)
	perf script -F +pid > pankti.perf
	


.PHONY: infer
infer: cmake_clean
	infer run --compilation-database build/compile_commands.json

.PHONY: clean
clean: cmake_clean
	rm -rf .zig-cache
	rm -rf .cache
	rm -rf zig-out


