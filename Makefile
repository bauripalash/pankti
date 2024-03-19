ZIG:=zig
BUILD_DIR:=zig-out
TARGET:=$(BUILD_DIR)/bin/pankti
SAMPLE:=sample/file.pank
DEBUGGER:=gdb
WASMBIN:=zig-out/bin/pankti.wasm

run: $(TARGET)
	@./$(TARGET) $(SAMPLE)

$(TARGET): build 


build:
	@$(ZIG) build -freference-trace


wasm:
	@$(ZIG) build wasm
	cp $(WASMBIN) ./web/

release:
	$(ZIG) build -Doptimize=ReleaseSafe

buildwin:
	$(ZIG) build -Dtarget=x86_64-windows -Doptimize=ReleaseSafe

fast:
	$(ZIG) build -Doptimize=ReleaseFast

debug: $(TARGET)
	$(DEBUGGER) --args $(TARGET) $(SAMPLE)

test:
	@$(ZIG) test src/main.zig

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
	rm -rf $(BUILD_DIR)

cleanall: clean
	rm -rf ./zig-cache/
