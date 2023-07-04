ZIG:=zig
BUILD_DIR:=zig-out
TARGET:=$(BUILD_DIR)/bin/neopank
SAMPLE:=sample/map.pank
DEBUGGER:=gdb

run: $(TARGET)
	@./$(TARGET) $(SAMPLE)

$(TARGET): build 


build:
	@$(ZIG) build 

wasm:
	$(ZIG) build-lib src/api.zig -target wasm32-freestanding -dynamic -rdynamic -freference-trace
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
	@echo "[+] Running Perf"
	perf record -g -F 999 ./$(TARGET) $(SAMPLE)
	perf script -F +pid > neopank.perf
	@echo "[+] Finished Running Perf"

clean:
	rm -rf $(BUILD_DIR)

cleanall: clean
	rm -rf ./zig-cache/
