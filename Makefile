ZIG:=zig
BUILD_DIR:=zig-out
TARGET:=$(BUILD_DIR)/bin/neopank
SAMPLE:=sample/g.pank
DEBUGGER:=gdb

run: $(TARGET)
	@./$(TARGET) $(SAMPLE)

$(TARGET): build 


build:
	@$(ZIG) build 

release:
	$(ZIG) build -Doptimize=ReleaseSafe

fast:
	$(ZIG) build -Doptimize=ReleaseFast

debug: $(TARGET)
	$(DEBUGGER) --args $(TARGET) $(SAMPLE)

test:
	@$(ZIG) test src/main.zig

clean:
	rm -rf $(BUILD_DIR)

cleanall: clean
	rm -rf ./zig-cache/
