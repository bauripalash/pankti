/*
 * Copyright (c) 2022 Palash Bauri
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PANKTI_PRINTER_H
#define PANKTI_PRINTER_H

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
