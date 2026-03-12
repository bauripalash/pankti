#include "include/native.h"
#include "include/env.h"
#include "include/gc.h"
#include "include/object.h"
#include "include/printer.h"
#include "include/pstdlib.h"
#include "include/ptypes.h"
#include "include/utils.h"
#include "include/vm.h"
#include <stddef.h>
#include <stdint.h>
#include <time.h>

#define NAME_CLOCK_EN      "clock"
#define NAME_CLOCK_BN      "সময়"
#define NAME_CLOCK_PN      "somoy"

#define NAME_SHOW_EN       "show"
#define NAME_SHOW_BN       "দেখাও"
#define NAME_SHOW_PN       "dekhao"

#define NAME_LEN_EN        "len"
#define NAME_LEN_BN        "আয়তন"
#define NAME_LEN_PN        "ayoton"

#define NAME_APPEND_EN     "append"
#define NAME_APPEND_BN     "সংযোগ"
#define NAME_APPEND_PN     "songjog"

#define NAME_ERROR_EN    "error"
#define NAME_ERROR_BN    "গোলমাল"
#define NAME_ERROR_PN    "golmal"

#define NATIVE_MODULE_NAME "সাধারণ"

static PValue ntvClock(PVm *vm, PValue *args, u64 argc) {
    clock_t c = clock();
    double sec = (double)c / (double)CLOCKS_PER_SEC;
    return MakeNumber(sec);
}

static PValue ntvShow(PVm *vm, PValue *args, u64 argc) {
    for (u64 i = 0; i < argc; i++) {
        PrintValue(args[i]);
        if (i + 1 < argc) {
            PanPrint(" ");
        }
    }
    return MakeNil();
}

static PValue ntvLen(PVm *vm, PValue *args, u64 argc) {
    PValue target = args[0];
    if (!IsValueObj(target)) {
        return MakeError(vm->gc, "Length can not be calculated for this value");
    }
    PObj *targetObj = ValueAsObj(target);
    if (!ObjectHasLen(targetObj)) {
        return MakeError(vm->gc, "Length can not be calculated for this value");
    }
    return MakeNumber(GetObjectLength(targetObj));
}

static PValue ntvAppend(PVm *vm, PValue *args, u64 argc) {

    if (argc < 2) {
        return MakeError(
            vm->gc,
            "append(...) needs atleast two arguments, for appending to array"
            " Or 3 arguments for adding new pair to a map"
        );
    }

    PValue target = args[0];

    if (IsValueObjType(target, OT_ARR)) {

        PObj *obj = ValueAsObj(target);

        u64 elmCount = argc - 1;
        for (u64 i = 0; i < elmCount; i++) {
            PValue elm = args[i + 1];
            if (!ArrayObjPushValue(obj, elm)) {
                return MakeError(
                    vm->gc, "Interenal Error : Failed to append item to array"
                );
            }
        }

        return MakeNumber((double)obj->v.OArray.count);
    } else if (IsValueObjType(target, OT_MAP)) {
        if (argc != 3) {
            return MakeError(
                vm->gc, "append(...) needs atleast 3 arguments, map and a "
                        "key-value pair"
            );
        }

        PObj *obj = ValueAsObj(target);
        PValue key = args[1];
        PValue value = args[2];

        if (!MapObjPushPair(obj, key, value, vm->gc->timestamp)) {
            return MakeError(
                vm->gc,
                "Interenal Error : Failed to append key-value pair to map"
            );
        }

        return MakeNumber((double)obj->v.OMap.count);
    }

    return MakeError(
        vm->gc, "append(...) function only works on arrays and maps"
    );
}


static PValue ntvError(PVm *vm, PValue *args, u64 argc) {
    PValue rawMsg = args[0];
    if (!IsValueObjType(rawMsg, OT_STR)) {
        return MakeError(vm->gc, "Error message must be a string");
    }
    struct OString * strObj = &ValueAsObj(rawMsg)->v.OString;

    //BUG: the error should own the message for gc's sake
    return MakeError(vm->gc, strObj->value);

}

#define DefStrHash(s, v) ((StrHash(s, DefStrLen(s), v->gc->timestamp)))

void RegisterNatives(PVm *vm, PEnv *env) {
    if (vm == NULL) {
        return;
    }

    if (vm->gc == NULL) {
        return;
    }

    StdlibEntry nativeEntries[] = {
        MakeStdlibEntry(NAME_CLOCK_EN, ntvClock, 0),
        MakeStdlibEntry(NAME_CLOCK_BN, ntvClock, 0),
        MakeStdlibEntry(NAME_CLOCK_PN, ntvClock, 0),
        MakeStdlibEntry(NAME_SHOW_EN, ntvShow, -1),
        MakeStdlibEntry(NAME_SHOW_BN, ntvShow, -1),
        MakeStdlibEntry(NAME_SHOW_PN, ntvShow, -1),
        MakeStdlibEntry(NAME_LEN_EN, ntvLen, 1),
        MakeStdlibEntry(NAME_LEN_BN, ntvLen, 1),
        MakeStdlibEntry(NAME_LEN_PN, ntvLen, 1),
        MakeStdlibEntry(NAME_APPEND_EN, ntvAppend, -1),
        MakeStdlibEntry(NAME_APPEND_BN, ntvAppend, -1),
        MakeStdlibEntry(NAME_APPEND_PN, ntvAppend, -1),
        MakeStdlibEntry(NAME_ERROR_EN, ntvError, 1),
        MakeStdlibEntry(NAME_ERROR_BN, ntvError, 1),
        MakeStdlibEntry(NAME_ERROR_PN, ntvError, 1),
    };

    u64 count = ArrCount(nativeEntries);
    PushStdlibEntries(
        vm, vm->globals, NATIVE_MODULE_NAME, nativeEntries, count
    );
}
