#ifndef PRINTER_H
#define PRINTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ptypes.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#ifndef PAN_PRINT_BUFFER_SIZE
#define PAN_PRINT_BUFFER_SIZE 8192 // 1024 * 8
#endif

bool InitPrintBuffer(void);
void DestroyPrintBuffer(void);

u64 PanFlushStdout(void);
u64 PanFlushStderr(void);

int PanPrint(const char *format, ...);
int PanFPrint(void *stream, const char *format, ...);
int PanVPrint(const char *format, va_list args);
int PanLog(const char *format, ...);
int PanFLog(void *stream, const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif
