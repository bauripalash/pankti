#include "../include/env.h"
#include "../include/interpreter.h"
#include "../include/pstdlib.h"
#include <math.h>
#include <stddef.h>
#include <string.h>

static inline double getGcd(double a, double b) {
    double x = (a > 0) ? a : -a;
    double y = (b > 0) ? b : -b;

    while (x != y) {
        if (x > y) {
            x = x - y;
        } else {
            y = y - x;
        }
    }

    return x;
}

static PValue math_Sqrt(PInterpreter *it, PValue *args, size_t argc) {
    PValue *rawVal = &args[0];
    if (rawVal->type != VT_NUM) {
        // MakeError
        return MakeError(it->gc, "Square root can only calculated for numbers");
    }
    double result = sqrt(rawVal->v.num);
    return MakeNumber(result);
}

static PValue math_Log10(PInterpreter *it, PValue *args, size_t argc) {
    PValue *rawVal = &args[0];
    if (rawVal->type != VT_NUM) {
        // MakeError
        return MakeNil();
    }
    double result = log10(rawVal->v.num);
    return MakeNumber(result);
}
static PValue math_Log(PInterpreter *it, PValue *args, size_t argc) {
    PValue *rawVal = &args[0];
    if (rawVal->type != VT_NUM) {
        // MakeError
        return MakeNil();
    }
    double result = log(rawVal->v.num);
    return MakeNumber(result);
}
static PValue math_LogBase(PInterpreter *it, PValue *args, size_t argc) {
    PValue *rawBase = &args[0];
    PValue *rawNum = &args[1];

    if (rawBase->type != VT_NUM || rawNum->type != VT_NUM) {
        // MakeError
        return MakeNil();
    }
    double result = log(rawNum->v.num) / log(rawBase->v.num);
    return MakeNumber(result);
}
static PValue math_GCD(PInterpreter *it, PValue *args, size_t argc) {
    return MakeNil();
}
static PValue math_LCM(PInterpreter *it, PValue *args, size_t argc) {
    return MakeNil();
}
static PValue math_Sine(PInterpreter *it, PValue *args, size_t argc) {
    return MakeNil();
}
static PValue math_Cosine(PInterpreter *it, PValue *args, size_t argc) {
    return MakeNil();
}
static PValue math_Tangent(PInterpreter *it, PValue *args, size_t argc) {
    return MakeNil();
}
static PValue math_Degree(PInterpreter *it, PValue *args, size_t argc) {
    return MakeNil();
}
static PValue math_Radians(PInterpreter *it, PValue *args, size_t argc) {
    return MakeNil();
}
static PValue math_Number(PInterpreter *it, PValue *args, size_t argc) {
    return MakeNil();
}
static PValue math_Abs(PInterpreter *it, PValue *args, size_t argc) {
    return MakeNil();
}
static PValue math_Round(PInterpreter *it, PValue *args, size_t argc) {
    return MakeNil();
}
static PValue math_Floor(PInterpreter *it, PValue *args, size_t argc) {
    return MakeNil();
}
static PValue math_Ceil(PInterpreter *it, PValue *args, size_t argc) {
    return MakeNil();
}
#define MATH_STD_SQRT    "বর্গমূল"
#define MATH_STD_LOGTEN  "লগদশ"
#define MATH_STD_LOG     "লগ"
#define MATH_STD_LOGBASE "লগবেস"
#define MATH_STD_GCD     "গসাগু"
#define MATH_STD_LCM     "লসাগু"
#define MATH_STD_SINE    "সাইন"
#define MATH_STD_COSINE  "কস"
#define MATH_STD_TANGENT "ট্যান"
#define MATH_STD_DEGREE  "ডিগ্রি"
#define MATH_STD_RADIANS "রেডিয়ান"
#define MATH_STD_NUM     "সংখ্যা"
#define MATH_STD_ABS     "পরম"
#define MATH_STD_ROUND   "রাউন্ড"
#define MATH_STD_FLOOR   "ফ্লোর"
#define MATH_STD_CEIL    "সিল"

void PushStdlibMath(PInterpreter *it, PEnv *env) {
    StdlibEntry entries[] = {
        MakeStdlibEntry(MATH_STD_SQRT, math_Sqrt, 1),
        MakeStdlibEntry(MATH_STD_LOGTEN, math_Log10, 1),
        MakeStdlibEntry(MATH_STD_LOG, math_Log, 1),
        MakeStdlibEntry(MATH_STD_LOGBASE, math_LogBase, 2),
        MakeStdlibEntry(MATH_STD_GCD, math_GCD, 2),
        MakeStdlibEntry(MATH_STD_LCM, math_LCM, 2),
        MakeStdlibEntry(MATH_STD_SINE, math_Sine, 1),
        MakeStdlibEntry(MATH_STD_COSINE, math_Cosine, 1),
        MakeStdlibEntry(MATH_STD_TANGENT, math_Tangent, 1),
        MakeStdlibEntry(MATH_STD_DEGREE, math_Degree, 1),
        MakeStdlibEntry(MATH_STD_RADIANS, math_Radians, 1),
        MakeStdlibEntry(MATH_STD_NUM, math_Number, 1),
        MakeStdlibEntry(MATH_STD_ABS, math_Abs, 1),
        MakeStdlibEntry(MATH_STD_ROUND, math_Round, 1),
        MakeStdlibEntry(MATH_STD_FLOOR, math_Floor, 1),
        MakeStdlibEntry(MATH_STD_CEIL, math_Ceil, 1)

    };
    int count = ArrCount(entries);
    for (int i = 0; i < count; i++) {
        const StdlibEntry *entry = &entries[i];
        PObj *stdFn = NewNativeFnObject(it->gc, NULL, entry->fn, entry->arity);
        EnvPutValue(
            env, StrHash(entry->name, entry->nlen, it->gc->timestamp),
            MakeObject(stdFn)
        );
    }

    EnvPutValue(env, StrHash("pi", 2, it->gc->timestamp), MakeNumber(CONST_PI));
    EnvPutValue(env, StrHash("e", 1, it->gc->timestamp), MakeNumber(CONST_E));
}
