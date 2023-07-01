ZIG:=zig
BUILD_DIR:=zig-out
TARGET:=$(BUILD_DIR)/bin/neopank
SAMPLE:=sample/fiben.pank
DEBUGGER:=gdb

run: $(TARGET)
	@./$(TARGET) $(SAMPLE)

$(TARGET): build 


build:
	@$(ZIG) build 

buildwasm:
	$(ZIG) build-lib src/api.zig -target wasm32-freestanding -dynamic -O ReleaseSafe -femit-bin=napi.wasm

release:
	$(ZIG) build -Doptimize=ReleaseSafe

fast:
	$(ZIG) build -Doptimize=ReleaseFast

debug: $(TARGET)
	$(DEBUGGER) --args $(TARGET) $(SAMPLE)

test:
	@$(ZIG) test src/main.zig

perf:
	@echo "[+] Running Perf"
	perf record -g -F 999 ./$(TARGET) $(SAMPLE)
	perf script -F +pid > neopank.perf
	@echo "[+] Finished Running Perf"

clean:
	rm -rf $(BUILD_DIR)

cleanall: clean
	rm -rf ./zig-cache/
