#include "include/printer.h"
#include "include/alloc.h"
#include "include/ptypes.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(PANKTI_OS_WIN)
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#endif

typedef struct PanBuffer {
    char *data;
    int len;
    int cap;
} PanBuffer;

static PanBuffer buffer = {0};
static bool hasBuffer = false;

bool InitPrintBuffer(void) {
    if (hasBuffer) {
        return true;
    }

    char *buf = (char *)PMalloc(sizeof(char) * PAN_PRINT_BUFFER_SIZE);
    if (buf == NULL) {
        return false;
    }

    buffer.data = buf;
    buffer.cap = PAN_PRINT_BUFFER_SIZE;
    buffer.len = 0;
    hasBuffer = true;
    return true;
}

void DestroyPrintBuffer(void) {
    if (!hasBuffer) {
        return;
    }

    PanFlushStdout();
    if (buffer.data != NULL) {
        PFree(buffer.data);
    }

    buffer.data = NULL;
    buffer.len = 0;
    buffer.cap = 0;
    hasBuffer = false;
}

static u64 panFlush(FILE *stream) {
    if (!hasBuffer || buffer.len == 0) {
        return 0;
    }

    u64 written = (u64)fwrite(buffer.data, 1, buffer.len, stream);
    fflush(stream);
    buffer.len = 0;
    return written;
}

u64 PanFlushStdout(void) { return panFlush(stdout); }

u64 PanFlushStderr(void) { return panFlush(stderr); }

int PanVPrint(const char *format, va_list args) {
    if (!hasBuffer) {
        return vprintf(format, args);
    }

    int remaining = buffer.cap - buffer.len;

    va_list argsCopy;
    va_copy(argsCopy, args);
    int needs = vsnprintf(
        buffer.data + buffer.len, (size_t)remaining, format, argsCopy
    );
    va_end(argsCopy);

    // something wrong happend
    // vsnprintf returns negetive numbers on error
    if (needs < 0) {
        return needs;
    }

    // the text easily fits in buffer
    if (needs < remaining) {
        buffer.len += needs;
        return needs;
    }

    // the buffer couldn't fit the text
    // clear the buffer then retry
    PanFlushStdout();

    // text is bigger than the whole buffer capacity
    // so just print it, we can't do anything here
    if (needs >= PAN_PRINT_BUFFER_SIZE) {
        return vprintf(format, args);
    }

    // now we are here that means
    // the text is smaller than buffer capacity
    // so it is gurrented to fit

    int result = vsnprintf(buffer.data, PAN_PRINT_BUFFER_SIZE, format, args);
    if (result > 0) {
        buffer.len = result;
    }

    return result;
}

int PanPrint(const char *format, ...) {
    va_list args;
    va_start(args, format);
    int result = PanVPrint(format, args);
    va_end(args);
    return result;
}

int PanFPrint(void *stream, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int result = vfprintf((FILE *)stream, format, args);
    va_end(args);
    return result;
}
