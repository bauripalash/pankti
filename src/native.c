#include "include/native.h"
#include "external/stb/stb_ds.h"
#include "include/compiler.h"
#include "include/core.h"
#include "include/gc.h"
#include "include/object.h"
#include "include/printer.h"
#include "include/pstdlib.h"
#include "include/ptypes.h"
#include "include/utils.h"
#include "include/vm.h"
#include <stdbool.h>
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

#define NAME_ERROR_EN      "error"
#define NAME_ERROR_BN      "গোলমাল"
#define NAME_ERROR_PN      "golmal"

#define NAME_ARGS_EN       "args"
#define NAME_ARGS_BN       "args"
#define NAME_ARGS_PN       "args"

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
        VmError(vm, "Length can not be calculated for this value");
        return MakeNil();
    }
    PObj *targetObj = ValueAsObj(target);
    if (!ObjectHasLen(targetObj)) {
        VmError(vm, "Length can not be calculated for this value");
        return MakeNil();
    }
    return MakeNumber(GetObjectLength(targetObj));
}

static PValue ntvAppend(PVm *vm, PValue *args, u64 argc) {

    if (argc < 2) {
        VmError(
            vm,
            "append(...) needs atleast two arguments, for appending to array"
            " Or 3 arguments for adding new pair to a map"
        );
        return MakeNil();
    }

    PValue target = args[0];

    if (IsValueObjType(target, OT_ARR)) {

        PObj *obj = ValueAsObj(target);

        u64 elmCount = argc - 1;
        for (u64 i = 0; i < elmCount; i++) {
            PValue elm = args[i + 1];
            if (!ArrayObjPushValue(obj, elm)) {
                VmError(vm, "Interenal Error : Failed to append item to array");
                return MakeNil();
            }
        }

        return MakeNumber((double)obj->v.OArray.count);
    } else if (IsValueObjType(target, OT_MAP)) {
        if (argc != 3) {
            VmError(
                vm, "append(...) needs atleast 3 arguments, map and a "
                    "key-value pair"
            );
            return MakeNil();
        }

        PObj *obj = ValueAsObj(target);
        PValue key = args[1];
        PValue value = args[2];

        if (!MapObjPushPair(obj, key, value, vm->gc->timestamp)) {
            VmError(
                vm, "Interenal Error : Failed to append key-value pair to map"
            );
            return MakeNil();
        }

        return MakeNumber((double)obj->v.OMap.count);
    }

    VmError(vm, "append(...) function only works on arrays and maps");
    return MakeNil();
}

static PValue ntvError(PVm *vm, PValue *args, u64 argc) {
    PValue rawMsg = args[0];
    if (!IsValueObjType(rawMsg, OT_STR)) {
        VmError(vm, "Error message must be a string");
        return MakeNil();
    }
    struct OString *strObj = &ValueAsObj(rawMsg)->v.OString;

    VmError(vm, strObj->value);
    return MakeNil();
}

static PValue ntvGetArgs(PVm *vm, PValue *args, u64 argc) {
    (void)args;
    (void)argc;
    return MakeNil();
    // BUG: VM should have reference to scripts args
    /*
    PanktiCore *core = vm->core;

    int sargCount = core->scriptArgCount;
    char **sargs = core->scriptArgs;

    if (sargCount == 0 || sargs == NULL) {
        PObj *emptyArray = NewArrayObject(vm->gc, NULL, NULL, 0);

        if (emptyArray == NULL) {
            VmError(vm, "Failed to create array for arguments list");
            return MakeNil();
        }

        return MakeObject(emptyArray);
    }

    PValue *items = NULL;
    for (int i = 0; i < sargCount; i++) {
        PObj *strObj = NewStrObject(vm->gc, NULL, sargs[i], false);
        if (strObj == NULL) {
            // handle error
        }

        arrpush(items, MakeObject(strObj));
    }

    PObj *arr = NewArrayObject(vm->gc, NULL, items, sargCount);

    return MakeObject(arr);
    */
}

#define DefStrHash(s, v) ((StrHash(s, DefStrLen(s), v->gc->timestamp)))

void RegisterNatives(PVm *vm) {
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
        MakeStdlibEntry(NAME_ARGS_EN, ntvGetArgs, -1),
        MakeStdlibEntry(NAME_ARGS_BN, ntvGetArgs, -1),
        MakeStdlibEntry(NAME_ARGS_PN, ntvGetArgs, -1),
    };

    u64 count = ArrCount(nativeEntries);
    PushStdlibEntries(
        vm, vm->globals, NATIVE_MODULE_NAME, nativeEntries, count
    );
}
