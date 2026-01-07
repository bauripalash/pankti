#include "include/native.h"
#include "include/env.h"
#include "include/gc.h"
// #include "include/interpreter.h"
#include "include/object.h"
#include "include/printer.h"
#include "include/ptypes.h"
#include "include/symtable.h"
#include "include/utils.h"
#include "include/vm.h"
#include <stddef.h>
#include <stdint.h>
#include <time.h>

#define NAME_CLOCK_EN  "clock"
#define NAME_CLOCK_BN  "সময়"
#define NAME_CLOCK_PN  "somoy"

#define NAME_SHOW_EN   "show"
#define NAME_SHOW_BN   "দেখাও"
#define NAME_SHOW_PN   "dekhao"

#define NAME_LEN_EN    "len"
#define NAME_LEN_BN    "আয়তন"
#define NAME_LEN_PN    "ayoton"

#define NAME_APPEND_EN "append"
#define NAME_APPEND_BN "সংযোগ"
#define NAME_APPEND_PN "songjog"

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

#define DefStrHash(s, v) ((StrHash(s, DefStrLen(s), v->gc->timestamp)))

void RegisterNatives(PVm *vm, PEnv *env) {
    if (vm == NULL) {
        return;
    }

    if (vm->gc == NULL) {
        return;
    }

    PObj *clockFn = NewNativeFnObject(vm->gc, NULL, ntvClock, 0);
    PObj *showFn = NewNativeFnObject(vm->gc, NULL, ntvShow, -1);
    PObj *lenFn = NewNativeFnObject(vm->gc, NULL, ntvLen, 1);
    PObj *appendFn = NewNativeFnObject(vm->gc, NULL, ntvAppend, -1);

    // todo: fix gc handling

    PObj *showFnEnStr = NewStrObject(vm->gc, NULL, NAME_SHOW_EN, false);
    PObj *showFnBnStr = NewStrObject(vm->gc, NULL, NAME_SHOW_BN, false);
    PObj *showFnPnStr = NewStrObject(vm->gc, NULL, NAME_SHOW_PN, false);

    PObj *appendFnEnStr = NewStrObject(vm->gc, NULL, NAME_APPEND_EN, false);
    PObj *appendFnBnStr = NewStrObject(vm->gc, NULL, NAME_APPEND_BN, false);
    PObj *appendFnPnStr = NewStrObject(vm->gc, NULL, NAME_APPEND_PN, false);

    SymbolTableSet(vm->globals, showFnEnStr, MakeObject(showFn));
    SymbolTableSet(vm->globals, showFnBnStr, MakeObject(showFn));
    SymbolTableSet(vm->globals, showFnPnStr, MakeObject(showFn));

    SymbolTableSet(vm->globals, appendFnEnStr, MakeObject(appendFn));
    SymbolTableSet(vm->globals, appendFnBnStr, MakeObject(appendFn));
    SymbolTableSet(vm->globals, appendFnPnStr, MakeObject(appendFn));

    /*
    EnvPutValue(env, DefStrHash(NAME_CLOCK_EN, vm), MakeObject(clockFn));
    EnvPutValue(env, DefStrHash(NAME_CLOCK_BN, vm), MakeObject(clockFn));
    EnvPutValue(env, DefStrHash(NAME_CLOCK_PN, vm), MakeObject(clockFn));

    EnvPutValue(env, DefStrHash(NAME_SHOW_EN, vm), MakeObject(showFn));
    EnvPutValue(env, DefStrHash(NAME_SHOW_BN, vm), MakeObject(showFn));
    EnvPutValue(env, DefStrHash(NAME_SHOW_PN, vm), MakeObject(showFn));

    EnvPutValue(env, DefStrHash(NAME_LEN_EN, vm), MakeObject(lenFn));
    EnvPutValue(env, DefStrHash(NAME_LEN_BN, vm), MakeObject(lenFn));
    EnvPutValue(env, DefStrHash(NAME_LEN_PN, vm), MakeObject(lenFn));

    EnvPutValue(env, DefStrHash(NAME_APPEND_EN, vm), MakeObject(appendFn));
    EnvPutValue(env, DefStrHash(NAME_APPEND_BN, vm), MakeObject(appendFn));
    EnvPutValue(env, DefStrHash(NAME_APPEND_PN, vm), MakeObject(appendFn));
    */
}
