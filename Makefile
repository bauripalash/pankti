ZIG:=zig
BUILD_DIR:=zig-out
CACHE_DIR:=zig-cache
TARGET:=$(BUILD_DIR)/bin/pankti
SAMPLE:=a.pank
DEBUGGER:=gdb
WASMBIN:=zig-out/bin/pankti.wasm
PYTHON:=python

run: $(TARGET)
	@./$(TARGET) $(SAMPLE)

$(TARGET): build 

ide:
	@$(ZIG) build ide
	@./$(BUILD_DIR)/bin/panktilekhok

capi:
	@$(ZIG) build api
	@cc -c apiexample.c
	@cc -o apiexample apiexample.o $(BUILD_DIR)/lib/libneopankapi.a -lm
	@./apiexample

build:
	@$(ZIG) build


wasm:
	@$(ZIG) build wasm
	cp $(WASMBIN) ./web/

release:
	$(ZIG) build -Doptimize=ReleaseSafe

buildwin:
	$(ZIG) build -Dtarget=x86_64-windows -Doptimize=ReleaseSafe

release_fast:
	$(ZIG) build -Doptimize=ReleaseFast

debug: $(TARGET)
	$(DEBUGGER) --args $(TARGET) $(SAMPLE)

test: $(TARGET)
	@$(ZIG) build test
	@$(PYTHON) -m unittest -v

resobj:
	llvm-rc winres/pankti.rc /FO winres/pankti.res.obj

perf:
	@echo "Building Release"
	$(ZIG) build -Doptimize=ReleaseSafe
	@echo "[+] Running Perf"
	perf record -g -F 999 ./$(TARGET) ./sample/fib35.pank
	perf script -F +pid > neopank.perf
	@echo "[+] Finished Running Perf"

clean:
	rm -rf $(CACHE_DIR)
	rm -rf $(BUILD_DIR)
