/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

// TODO: Sine, Cos, Tangent type functions should take degrees instead of
// radians for simpler mathematical calculations
#include "../include/gc.h"
#include "../include/pstdlib.h"
#include "../include/ptypes.h"
#include "../include/vm.h"
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

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

static inline double toRad(double deg) { return deg * (CONST_PI / 180.0); }

static inline double getRandNumRange(double min, double max) {
    double scale = (double)rand() / (double)RAND_MAX;
    return min + scale * (max - min);
}

static PValue math_Sqrt(PVm *vm, PValue *args, u64 argc) {
    PValue rawVal = args[0];
    if (!IsValueNum(rawVal)) {
        VmError(vm, RT_STDMATH_SQRT_NOT_NUM, ValueTypeToStr(rawVal));
        return MakeNil();
    }
    double result = sqrt(ValueAsNum(rawVal));
    return MakeNumber(result);
}

static PValue math_Log10(PVm *vm, PValue *args, u64 argc) {
    PValue rawVal = args[0];
    if (!IsValueNum(rawVal)) {
        VmError(vm, RT_STDMATH_LOG10_NOT_NUM, ValueTypeToStr(rawVal));
        return MakeNil();
    }
    double result = log10(ValueAsNum(rawVal));
    return MakeNumber(result);
}
static PValue math_Log(PVm *vm, PValue *args, u64 argc) {
    PValue rawVal = args[0];
    if (!IsValueNum(rawVal)) {
        VmError(vm, RT_STDMATH_LOG_NOT_NUM, ValueTypeToStr(rawVal));
        return MakeNil();
    }
    double result = log(ValueAsNum(rawVal));
    return MakeNumber(result);
}
static PValue math_LogBase(PVm *vm, PValue *args, u64 argc) {
    PValue rawBase = args[0];
    PValue rawNum = args[1];

    if (!IsValueNum(rawBase)) {
        VmError(vm, RT_STDMATH_LOGBASE_BASE_NOT_NUM, ValueTypeToStr(rawBase));
        return MakeNil();
    }

    if (!IsValueNum(rawNum)) {
        VmError(vm, RT_STDMATH_LOGBASE_NUMBER_NOT_NUM, ValueTypeToStr(rawNum));
        return MakeNil();
    }
    double result = log(ValueAsNum(rawNum)) / log(ValueAsNum(rawBase));
    return MakeNumber(result);
}
static PValue math_GCD(PVm *vm, PValue *args, u64 argc) {
    PValue rawA = args[0];
    PValue rawB = args[1];

    if (!IsValueNum(rawA)) {
        VmError(vm, RT_STDMATH_GCD_A_NOT_NUM, ValueTypeToStr(rawA));
        return MakeNil();
    }

    if (!IsValueNum(rawB)) {
        VmError(vm, RT_STDMATH_GCD_B_NOT_NUM, ValueTypeToStr(rawB));
        return MakeNil();
    }

    double result = getGcd(ValueAsNum(rawA), ValueAsNum(rawB));
    return MakeNumber(result);
}
static PValue math_LCM(PVm *vm, PValue *args, u64 argc) {
    PValue rawA = args[0];
    PValue rawB = args[1];

    if (!IsValueNum(rawA)) {
        VmError(vm, RT_STDMATH_LCM_A_NOT_NUM, ValueTypeToStr(rawA));
        return MakeNil();
    }
    if (!IsValueNum(rawB)) {
        VmError(vm, RT_STDMATH_LCM_B_NOT_NUM, ValueTypeToStr(rawB));
        return MakeNil();
    }

    double numA = ValueAsNum(rawA);
    double numB = ValueAsNum(rawB);
    double result = (numA * numB) / getGcd(numA, numB);
    return MakeNumber(result);
}
static PValue math_Sine(PVm *vm, PValue *args, u64 argc) {
    PValue raw = args[0];
    if (!IsValueNum(raw)) {
        VmError(vm, RT_STDMATH_SIN_NOT_NUM, ValueTypeToStr(raw));
        return MakeNil();
    }
    double target = toRad(ValueAsNum(raw));
    double result = sin(target);
    return MakeNumber(result);
}
static PValue math_Cosine(PVm *vm, PValue *args, u64 argc) {
    PValue raw = args[0];
    if (!IsValueNum(raw)) {
        VmError(vm, RT_STDMATH_COS_NOT_NUM, ValueTypeToStr(raw));
        return MakeNil();
    }
    double target = toRad(ValueAsNum(raw));
    double result = cos(target);
    return MakeNumber(result);
}
static PValue math_Tangent(PVm *vm, PValue *args, u64 argc) {
    PValue raw = args[0];
    if (!IsValueNum(raw)) {
        VmError(vm, RT_STDMATH_TAN_NOT_NUM, ValueTypeToStr(raw));
        return MakeNil();
    }
    double degValue = ValueAsNum(raw);

    // tan result invalid on odd multiples of 90
    double norm = fmod(fabs(degValue), 180.0);
    if (norm == 90.0) {
        VmError(vm, RT_STDMATH_TAN_RESULT_INVALID, degValue);
        return MakeNil();
    }

    if (norm == 0.0) return MakeNumber(0.0);

    double target = toRad(degValue);
    double result = tan(target);

    return MakeNumber(result);
}
static PValue math_Degree(PVm *vm, PValue *args, u64 argc) {
    PValue raw = args[0];
    if (!IsValueNum(raw)) {
        VmError(vm, RT_STDMATH_DEG_NOT_NUM, ValueTypeToStr(raw));
        return MakeNil();
    }
    double result = ValueAsNum(raw) * (180.0 / CONST_PI);
    return MakeNumber(result);
}
static PValue math_Radians(PVm *vm, PValue *args, u64 argc) {
    PValue raw = args[0];
    if (!IsValueNum(raw)) {
        VmError(vm, RT_STDMATH_RAD_NOT_NUM, ValueTypeToStr(raw));
        return MakeNil();
    }
    double result = ValueAsNum(raw) * (CONST_PI / 180.0);
    return MakeNumber(result);
}
static PValue math_Number(PVm *vm, PValue *args, u64 argc) {
    PValue rawStr = args[0];

    if (!IsValueObjType(rawStr, OT_STR)) {
        VmError(vm, RT_STDMATH_NUMBER_NOT_NUM, ValueTypeToStr(rawStr));
        return MakeNil();
    }

    struct OString *strObj = &ValueAsObj(rawStr)->v.OString;
    double result = 0;
    bool isok = true;
    result = NumberFromStr(strObj->value, StrLength(strObj->value), &isok);

    if (!isok) {
        VmError(vm, RT_IME_STDMATH_NUMBER_CONVERT_FAIL);
        return MakeNil();
    }

    return MakeNumber(result);
}
static PValue math_Abs(PVm *vm, PValue *args, u64 argc) {
    PValue raw = args[0];
    if (!IsValueNum(raw)) {
        VmError(vm, RT_STDMATH_ABS_NOT_NUM, ValueTypeToStr(raw));
        return MakeNil();
    }
    double result = fabs(ValueAsNum(raw));
    return MakeNumber(result);
}
static PValue math_Round(PVm *vm, PValue *args, u64 argc) {
    PValue raw = args[0];
    if (!IsValueNum(raw)) {
        VmError(vm, RT_STDMATH_ROUND_NOT_NUM, ValueTypeToStr(raw));
        return MakeNil();
    }
    double result = round(ValueAsNum(raw));
    return MakeNumber(result);
}
static PValue math_Floor(PVm *vm, PValue *args, u64 argc) {
    PValue raw = args[0];
    if (!IsValueNum(raw)) {
        VmError(vm, RT_STDMATH_FLOOR_NOT_NUM, ValueTypeToStr(raw));
        return MakeNil();
    }
    double result = floor(ValueAsNum(raw));
    return MakeNumber(result);
}
static PValue math_Ceil(PVm *vm, PValue *args, u64 argc) {
    PValue raw = args[0];
    if (!IsValueNum(raw)) {
        VmError(vm, RT_STDMATH_CEIL_NOT_NUM, ValueTypeToStr(raw));
        return MakeNil();
    }
    double result = ceil(ValueAsNum(raw));
    return MakeNumber(result);
}

static PValue math_Random(PVm *vm, PValue *args, u64 argc) {
    PValue rawMin = args[0];
    PValue rawMax = args[1];

    if (!IsValueNum(rawMin)) {
        VmError(vm, RT_STDMATH_RANDOM_MIN_NOT_NUM, ValueTypeToStr(rawMin));
        return MakeNil();
    }

    if (!IsValueNum(rawMax)) {
        VmError(vm, RT_STDMATH_RANDOM_MAX_NOT_NUM, ValueTypeToStr(rawMax));
        return MakeNil();
    }

    double result = getRandNumRange(ValueAsNum(rawMin), ValueAsNum(rawMax));
    return MakeNumber(result);
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
#define MATH_STD_ROUND   "নিকটতম"
#define MATH_STD_FLOOR   "নিকট_ছোট"
#define MATH_STD_CEIL    "নিকট_বড়"
#define MATH_STD_PI      "পাই"
#define MATH_STD_E       "ই"
#define MATH_STD_RANDOM  "এলোমেলো"

void PushStdlibMath(PVm *vm, SymbolTable *table) {
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
        MakeStdlibEntry(MATH_STD_CEIL, math_Ceil, 1),
        MakeStdlibEntry(MATH_STD_RANDOM, math_Random, 2)

    };
    int count = ArrCount(entries);

    PushStdlibEntries(vm, table, MATH_STDLIB_NAME, entries, count);
    PObj *piNameObj = NewStrObject(vm->gc, NULL, MATH_STD_PI, false);
    if (piNameObj == NULL) {
        VmError(vm, RT_IME_STDMATH_PI_STR);
        return;
    }
    VmPush(vm, MakeObject(piNameObj));
    PObj *eNameObj = NewStrObject(vm->gc, NULL, MATH_STD_E, false);
    if (eNameObj == NULL) {
        VmError(vm, RT_IME_STDMATH_E_STR);
        return;
    }
    VmPush(vm, MakeObject(eNameObj));
    SymbolTableSet(table, piNameObj, MakeNumber(CONST_PI));
    SymbolTableSet(table, eNameObj, MakeNumber(CONST_E));
    VmPop(vm);
    VmPop(vm);
}
