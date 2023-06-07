ZIG:=zig
BUILD_DIR:=zig-out
TARGET:=$(BUILD_DIR)/bin/neopank
SAMPLE:=sample/c.txt

run: $(TARGET)
	./$(TARGET) $(SAMPLE)

$(TARGET): build 


build:
	$(ZIG) build 

test:
	@$(ZIG) test src/main.zig

clean:
	rm -rf $(BUILD_DIR)

cleanall: clean
	rm -rf ./zig-cache/
