#include "../include/gc.h"
#include "../include/pstdlib.h"
#include "../include/ptypes.h"
#include "../include/vm.h"
#include <math.h>
#include <stdbool.h>
#include <stddef.h>

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

static PValue math_Sqrt(PVm *vm, PValue *args, u64 argc) {
    PValue rawVal = args[0];
    if (!IsValueNum(rawVal)) {
        // MakeError
        return MakeError(vm->gc, "Square root can only calculated for numbers");
    }
    double result = sqrt(ValueAsNum(rawVal));
    return MakeNumber(result);
}

static PValue math_Log10(PVm *vm, PValue *args, u64 argc) {
    PValue rawVal = args[0];
    if (!IsValueNum(rawVal)) {
        // MakeError
        return MakeNil();
    }
    double result = log10(ValueAsNum(rawVal));
    return MakeNumber(result);
}
static PValue math_Log(PVm *vm, PValue *args, u64 argc) {
    PValue rawVal = args[0];
    if (!IsValueNum(rawVal)) {
        // MakeError
        return MakeNil();
    }
    double result = log(ValueAsNum(rawVal));
    return MakeNumber(result);
}
static PValue math_LogBase(PVm *vm, PValue *args, u64 argc) {
    PValue rawBase = args[0];
    PValue rawNum = args[1];

    if (!IsValueNum(rawBase) || !IsValueNum(rawNum)) {
        // MakeError
        return MakeNil();
    }
    double result = log(ValueAsNum(rawNum)) / log(ValueAsNum(rawBase));
    return MakeNumber(result);
}
static PValue math_GCD(PVm *vm, PValue *args, u64 argc) {
    PValue rawA = args[0];
    PValue rawB = args[1];

    if (!IsValueNum(rawA) || !IsValueNum(rawB)) {
        // Make Error
        return MakeNil();
    }

    double result = getGcd(ValueAsNum(rawA), ValueAsNum(rawB));
    return MakeNumber(result);
}
static PValue math_LCM(PVm *vm, PValue *args, u64 argc) {
    PValue rawA = args[0];
    PValue rawB = args[1];

    if (!IsValueNum(rawA) || !IsValueNum(rawB)) {
        // Make Error
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
        return MakeNil(); // error
    }
    double result = sin(ValueAsNum(raw));
    return MakeNumber(result);
}
static PValue math_Cosine(PVm *vm, PValue *args, u64 argc) {
    PValue raw = args[0];
    if (!IsValueNum(raw)) {
        return MakeNil(); // error
    }
    double result = cos(ValueAsNum(raw));
    return MakeNumber(result);
}
static PValue math_Tangent(PVm *vm, PValue *args, u64 argc) {
    PValue raw = args[0];
    if (!IsValueNum(raw)) {
        return MakeNil(); // error
    }
    double result = tan(ValueAsNum(raw));
    return MakeNumber(result);
}
static PValue math_Degree(PVm *vm, PValue *args, u64 argc) {
    PValue raw = args[0];
    if (!IsValueNum(raw)) {
        return MakeNil(); // error
    }
    double result = ValueAsNum(raw) * (180.0 / CONST_PI);
    return MakeNumber(result);
}
static PValue math_Radians(PVm *vm, PValue *args, u64 argc) {
    PValue raw = args[0];
    if (!IsValueNum(raw)) {
        return MakeNil(); // error
    }
    double result = ValueAsNum(raw) * (CONST_PI / 180.0);
    return MakeNumber(result);
}
static PValue math_Number(PVm *vm, PValue *args, u64 argc) {
    PValue rawStr = args[0];

    if (!IsValueObjType(rawStr, OT_STR)) {
        return MakeError(vm->gc, "Only string can be converted to numbers");
    }

    struct OString *strObj = &ValueAsObj(rawStr)->v.OString;
    double result = 0;
    bool isok = true;
    if (strObj->isVirtual) {
        result = NumberFromStr(strObj->value, StrLength(strObj->value), &isok);
    } else {
        result = NumberFromStr(strObj->name->lexeme, strObj->name->len, &isok);
    }

    if (!isok) {
        return MakeError(vm->gc, "Failed to convert string to number");
    }

    return MakeNumber(result);
}
static PValue math_Abs(PVm *vm, PValue *args, u64 argc) {
    PValue raw = args[0];
    if (!IsValueNum(raw)) {
        return MakeNil(); // error
    }
    double result = fabs(ValueAsNum(raw));
    return MakeNumber(result);
}
static PValue math_Round(PVm *vm, PValue *args, u64 argc) {
    PValue raw = args[0];
    if (!IsValueNum(raw)) {
        return MakeNil(); // error
    }
    double result = round(ValueAsNum(raw));
    return MakeNumber(result);
}
static PValue math_Floor(PVm *vm, PValue *args, u64 argc) {
    PValue raw = args[0];
    if (!IsValueNum(raw)) {
        return MakeNil(); // error
    }
    double result = floor(ValueAsNum(raw));
    return MakeNumber(result);
}
static PValue math_Ceil(PVm *vm, PValue *args, u64 argc) {
    PValue raw = args[0];
    if (!IsValueNum(raw)) {
        return MakeNil(); // error
    }
    double result = ceil(ValueAsNum(raw));
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
#define MATH_STD_ROUND   "রাউন্ড"
#define MATH_STD_FLOOR   "ফ্লোর"
#define MATH_STD_CEIL    "সিল"
#define MATH_STD_PI      "পাই"
#define MATH_STD_E       "ই"

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
        MakeStdlibEntry(MATH_STD_CEIL, math_Ceil, 1)

    };
    int count = ArrCount(entries);

    PushStdlibEntries(vm, table, entries, count);
    PObj *piNameObj = NewStrObject(vm->gc, NULL, MATH_STD_PI, false);
    VmPush(vm, MakeObject(piNameObj));
    PObj *eNameObj = NewStrObject(vm->gc, NULL, MATH_STD_E, false);
    VmPush(vm, MakeObject(eNameObj));
    SymbolTableSet(table, piNameObj, MakeNumber(CONST_PI));
    SymbolTableSet(table, eNameObj, MakeNumber(CONST_E));
    VmPop(vm);
    VmPop(vm);
}
