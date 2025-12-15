#ifndef PRINTER_H
#define PRINTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdarg.h>

int PanPrint(const char * format, ...);
int PanFPrint(void * stream, const char * format, ...);
int PanVPrint(const char * format, va_list args);
int PanLog(const char * format, ...);
int PanFLog(void * stream, const char * format, ...);

#ifdef __cplusplus
}
#endif

#endif
