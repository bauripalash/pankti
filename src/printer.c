#include "include/printer.h"
#include <stdarg.h>
#include <stdio.h>

#if defined(PANKTI_OS_WIN)
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#endif

int PanPrint(const char *format, ...) {
    va_list args;
    va_start(args, format);
    int result = vprintf(format, args);
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

int PanVPrint(const char *format, va_list args) {
    return vprintf(format, args);
}
