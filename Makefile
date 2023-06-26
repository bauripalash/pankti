ZIG:=zig
BUILD_DIR:=zig-out
TARGET:=$(BUILD_DIR)/bin/neopank
SAMPLE:=sample/e.pank
DEBUGGER:=gdb

run: $(TARGET)
	./$(TARGET) $(SAMPLE)

$(TARGET): build 


build:
	$(ZIG) build 

debug: $(TARGET)
	$(DEBUGGER) --args $(TARGET) $(SAMPLE)

test:
	@$(ZIG) test src/main.zig

clean:
	rm -rf $(BUILD_DIR)

cleanall: clean
	rm -rf ./zig-cache/
