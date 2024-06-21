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

release: rls_setup rls_win32 rls_win64 rls_linux32 rls_linux64

build:
	@$(ZIG) build


wasm:
	@$(ZIG) build wasm
	cp $(WASMBIN) ./web/


debug: $(TARGET)
	$(DEBUGGER) --args $(TARGET) $(SAMPLE)

test: $(TARGET)
	@$(ZIG) build test
	@$(PYTHON) -m unittest -v


rls_setup:
	mkdir -p dist/

rls_win32:
	$(ZIG) build -Dtarget=x86-windows-gnu --release=safe
	mv zig-out/bin/pankti.exe dist/pankti-win32.exe

rls_win64:
	$(ZIG) build -Dtarget=x86_64-windows-gnu --release=safe
	mv zig-out/bin/pankti.exe dist/pankti-win64.exe

rls_linux32:
	$(ZIG) build -Dtarget=x86-linux-gnu --release=safe
	mv zig-out/bin/pankti dist/pankti-linux32

rls_linux64:
	$(ZIG) build -Dtarget=x86_64-linux-gnu --release=safe
	mv zig-out/bin/pankti dist/pankti-linux64

resobj:
	$(ZIG) rc winres/pankti.rc /FO winres/pankti.res.obj

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
