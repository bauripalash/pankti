ZIG:=zig
BUILD_DIR:=zig-out
CACHE_DIR:=zig-cache
TARGET:=$(BUILD_DIR)/bin/pankti
SAMPLE:=a.pank
DEBUGGER:=gdb
WASMBIN:=zig-out/bin/pankti.wasm
PYTHON:=python
ICON_ICO:=images/icon.ico

ifeq ($(PREFIX),)
	PREFIX:=/usr/local
endif

run: $(TARGET)
	@./$(TARGET) $(SAMPLE)

$(TARGET): build 

r: $(TARGET)
	@./$(TARGET) $(ARGS)

release: rls_setup rls_win32 rls_win64 rls_linux32 rls_linux64

build:
	@$(ZIG) build


wasm:
	@$(ZIG) build wasm -Dtarget=wasm32-freestanding-musl -Doptimize=ReleaseFast
	cp $(WASMBIN) ./web/


debug: $(TARGET)
	$(DEBUGGER) --args $(TARGET) $(SAMPLE)

test: $(TARGET)
	@$(ZIG) build test
	@$(PYTHON) -m unittest -v


rls_setup:
	mkdir -p dist/

rls_win32:
	$(ZIG) build -Dtarget=x86-windows-gnu -Doptimize=ReleaseSafe
	mv zig-out/bin/pankti.exe dist/pankti-win32.exe

rls_win64:
	$(ZIG) build -Dtarget=x86_64-windows-gnu -Doptimize=ReleaseSafe
	mv zig-out/bin/pankti.exe dist/pankti-win64.exe

rls_linux32:
	$(ZIG) build -Dtarget=x86-linux-gnu -Doptimize=ReleaseSafe
	mv zig-out/bin/pankti dist/pankti-linux32

rls_linux64:
	$(ZIG) build -Dtarget=x86_64-linux-gnu -Doptimize=ReleaseSafe
	mv zig-out/bin/pankti dist/pankti-linux64

resobj:
	$(ZIG) rc winres/pankti.rc /FO winres/pankti.res.obj

perf:
	@echo "Building Release"
	$(ZIG) build -Doptimize=ReleaseFast
	@echo "[+] Running Perf"
	perf record -g -F 999 ./$(TARGET) ./sample/fib35.pank
	perf script -F +pid > neopank.perf
	@echo "[+] Finished Running Perf"


install: $(TARGET)
	install -d $(DESTDIR)$(PREFIX)/bin
	install -d $(DESTDIR)$(PREFIX)/share/
	install -d $(DESTDIR)$(PREFIX)/share/pankti/
	install -d $(DESTDIR)$(PREFIX)/share/pankti/icons/
	install -m 755 $(TARGET) $(DESTDIR)$(PREFIX)/bin
	install -m 644 $(ICON_ICO) $(DESTDIR)$(PREFIX)/share/pankti/icons/

uninstall:
	rm $(DESTDIR)$(PREFIX)/bin/pankti
	rm -rf $(DESTDIR)$(PREFIX)/share/pankti

fmt:
	@find . -path ./.zig-cache -prune -o -name "*.zig"\
		-exec $(ZIG) fmt {} \;

clean:
	rm -rf $(CACHE_DIR)
	rm -rf $(BUILD_DIR)
	rm -rf ./src/ext/baurinum/.zig-cache
	rm -rf dist/
