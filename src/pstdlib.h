/*
 * Copyright (c) 2022 Palash Bauri
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PANKTI_PSTDLIB_H
#define PANKTI_PSTDLIB_H

#include "object.h"
#include "symtable.h"
#include "utils.h"
#include <stdbool.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

#define SYSTEM_STDLIB_NAME "পরিবেশ"
#define MATH_STDLIB_NAME   "গণিত"
#define MAP_STDLIB_NAME    "ছক"
#define STRING_STDLIB_NAME "কথা"
#define ARRAY_STDLIB_NAME  "তালিকা"
#define GFX_STDLIB_NAME    "পট"
#define FILE_STDLIB_NAME   "নথি"

typedef enum StdlibMod {
    STDLIB_NONE = 0,
    STDLIB_SYSTEM,
    STDLIB_MATH,
    STDLIB_MAP,
    STDLIB_ARRAY,
    STDLIB_STRING,
    STDLIB_GRAPHICS,
    STDLIB_FILE,
} StdlibMod;

typedef struct StdlibEntry {
    char *name;
    u64 nlen;
    NativeFn fn;
    int arity;
} StdlibEntry;

#define MakeStdlibEntry(sname, nfn, ar)                                        \
    ((StdlibEntry){                                                            \
        .name = sname, .nlen = (u64)strlen(sname), .fn = nfn, .arity = ar      \
    })

typedef struct PVm PVm;

static inline StdlibMod GetStdlibMod(const char *name) {
    if (StrEqual(name, SYSTEM_STDLIB_NAME)) {
        return STDLIB_SYSTEM;
    } else if (StrEqual(name, MATH_STDLIB_NAME)) {
        return STDLIB_MATH;
    } else if (StrEqual(name, MAP_STDLIB_NAME)) {
        return STDLIB_MAP;
    } else if (StrEqual(name, ARRAY_STDLIB_NAME)) {
        return STDLIB_ARRAY;
    } else if (StrEqual(name, STRING_STDLIB_NAME)) {
        return STDLIB_STRING;
    } else if (StrEqual(name, GFX_STDLIB_NAME)) {
#if defined(HAS_GFX_SUPPORT) && !defined(NO_GFX_SUPPORT)
        return STDLIB_GRAPHICS;
#else
        return STDLIB_NONE;
#endif
    } else if (StrEqual(name, FILE_STDLIB_NAME)) {
        return STDLIB_FILE;
    }
    return STDLIB_NONE;
}

void PushStdlib(PVm *vm, SymbolTable *table, const char *name, StdlibMod mod);
void PushStdlibSystem(PVm *vm, SymbolTable *table);
void PushStdlibMath(PVm *vm, SymbolTable *table);
void PushStdlibMap(PVm *vm, SymbolTable *table);
void PushStdlibArray(PVm *vm, SymbolTable *table);
void PushStdlibString(PVm *vm, SymbolTable *table);
#if defined(HAS_GFX_SUPPORT) && !defined(NO_GFX_SUPPORT)
void PushStdlibGraphics(PVm *vm, SymbolTable *table);
#else
static inline void PushStdlibGraphics(PVm *vm, SymbolTable *table) { return; }
#endif
void PushStdlibFile(PVm *vm, SymbolTable *table);
void PushStdlibEntries(
    PVm *vm,
    SymbolTable *table,
    const char *module,
    StdlibEntry *entries,
    u64 count
);

#ifdef __cplusplus
}
#endif

#endif
