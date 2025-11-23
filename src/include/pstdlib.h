#ifndef P_STDLIB_H
#define P_STDLIB_H

#include "env.h"
#include "object.h"
#include "ptypes.h"
#include "utils.h"
#include <stdbool.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

#define OS_STDLIB_NAME     "os"
#define MATH_STDLIB_NAME   "গণিত"
#define MAP_STDLIB_NAME    "map"
#define STRING_STDLIB_NAME "string"

typedef enum StdlibMod {
    STDLIB_NONE = 0,
    STDLIB_OS,
    STDLIB_MATH,
    STDLIB_MAP,
    STDLIB_STRING,
} StdlibMod;

typedef struct StdlibEntry {
    char *name;
    pusize nlen;
    NativeFn fn;
    pint arity;
} StdlibEntry;


#define MakeStdlibEntry(sname, nfn, ar) ((StdlibEntry){.name = sname, .nlen = (pusize)strlen(sname), .fn = nfn, .arity = ar})

typedef struct PInterpreter PInterpreter;

static inline StdlibMod GetStdlibMod(const char *name) {
    if (StrEqual(name, OS_STDLIB_NAME)) {
        return STDLIB_OS;
    } else if (StrEqual(name, MATH_STDLIB_NAME)) {
        return STDLIB_MATH;
    } else if (StrEqual(name, MAP_STDLIB_NAME)) {
        return STDLIB_MAP;
    } else if (StrEqual(name, STRING_STDLIB_NAME)) {
        return STDLIB_STRING;
    }
    return STDLIB_NONE;
}

void PushStdlib(PInterpreter *it, PEnv *env, const char *name, StdlibMod mod);
void PushStdlibOs(PInterpreter *it, PEnv *env);
void PushStdlibMath(PInterpreter *it, PEnv *env);
void PushStdlibMap(PInterpreter *it, PEnv *env);
void PushStdlibString(PInterpreter *it, PEnv *env);

#ifdef __cplusplus
}
#endif

#endif
