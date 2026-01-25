#ifndef P_STDLIB_H
#define P_STDLIB_H

#include "object.h"
#include "symtable.h"
#include "utils.h"
#include <stdbool.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

#define OS_STDLIB_NAME     "ওএস"
#define MATH_STDLIB_NAME   "গণিত"
#define MAP_STDLIB_NAME    "ম্যাপ"
#define STRING_STDLIB_NAME "string"
#define ARRAY_STDLIB_NAME  "তালিকা"
#define GFX_STDLIB_NAME    "gfx"

typedef enum StdlibMod {
    STDLIB_NONE = 0,
    STDLIB_OS,
    STDLIB_MATH,
    STDLIB_MAP,
    STDLIB_ARRAY,
    STDLIB_STRING,
    STDLIB_GRAPHICS,
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
    if (StrEqual(name, OS_STDLIB_NAME)) {
        return STDLIB_OS;
    } else if (StrEqual(name, MATH_STDLIB_NAME)) {
        return STDLIB_MATH;
    } else if (StrEqual(name, MAP_STDLIB_NAME)) {
        return STDLIB_MAP;
    } else if (StrEqual(name, ARRAY_STDLIB_NAME)) {
        return STDLIB_ARRAY;
    } else if (StrEqual(name, STRING_STDLIB_NAME)) {
        return STDLIB_STRING;
    } else if (StrEqual(name, GFX_STDLIB_NAME)) {
#if defined (HAS_GFX_SUPPORT) && !defined(NO_GFX_SUPPORT)
        return STDLIB_GRAPHICS;
#else
        return STDLIB_NONE;
#endif
    }
    return STDLIB_NONE;
}

void PushStdlib(PVm *vm, SymbolTable *table, const char *name, StdlibMod mod);
void PushStdlibOs(PVm *vm, SymbolTable *table);
void PushStdlibMath(PVm *vm, SymbolTable *table);
void PushStdlibMap(PVm *vm, SymbolTable *table);
void PushStdlibArray(PVm *vm, SymbolTable *table);
void PushStdlibString(PVm *vm, SymbolTable *table);
#if defined (HAS_GFX_SUPPORT) && !defined(NO_GFX_SUPPORT)
void PushStdlibGraphics(PVm *vm, SymbolTable *table);
#else
static inline void PushStdlibGraphics(PVm *vm, SymbolTable *table){
    return;
}
#endif
void PushStdlibEntries(
    PVm *vm, SymbolTable *table, StdlibEntry *entries, u64 count
);

#ifdef __cplusplus
}
#endif

#endif
