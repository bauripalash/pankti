/*
 * Copyright (c) 2022 Palash Bauri
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "include/builtins.h"
#include "external/stb/stb_ds.h"
#include "gen/diagon.h"
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

#define NAME_CLOCK_EN       "clock"
#define NAME_CLOCK_BN       "সময়"
#define NAME_CLOCK_PN       "somoy"

#define NAME_SHOW_EN        "show"
#define NAME_SHOW_BN        "দেখাও"
#define NAME_SHOW_PN        "dekhao"

#define NAME_TYPE_EN        "type"
#define NAME_TYPE_BN        "প্রকার"
#define NAME_TYPE_PN        "prokar"

#define NAME_LEN_EN         "len"
#define NAME_LEN_BN         "আয়তন"
#define NAME_LEN_PN         "ayoton"

#define NAME_APPEND_EN      "append"
#define NAME_APPEND_BN      "সংযোগ"
#define NAME_APPEND_PN      "songjog"

#define NAME_ERROR_EN       "error"
#define NAME_ERROR_BN       "গোলমাল"
#define NAME_ERROR_PN       "golmal"

#define NAME_ARGS_EN        "args"
#define NAME_ARGS_BN        "প্রেরণমান"
#define NAME_ARGS_PN        "preronman"

#define BUILTIN_MODULE_NAME "সাধারণ"

static PValue builtinClock(PVm *vm, PValue *args, u64 argc) {
    clock_t c = clock();
    double sec = (double)c / (double)CLOCKS_PER_SEC;
    return MakeNumber(sec);
}

static PValue builtinShow(PVm *vm, PValue *args, u64 argc) {
    for (u64 i = 0; i < argc; i++) {
        PrintValue(args[i]);
        if (i + 1 < argc) {
            PanPrint(" ");
        }
    }
    return MakeNil();
}

static PValue builtinType(PVm *vm, PValue *args, u64 argc) {
    PValue target = args[0];
    const char *typeName = ValueTypeToStr(target);
    char *typeNameMlcd = StrDuplicate(typeName, StrLength(typeName));
    if (typeNameMlcd == NULL) {
        VmError(vm, RT_IME_BUILTIN_TYPE_STRDUP);
        return MakeNil();
    }
    PObj *typeStrObj = NewStrObject(vm->gc, NULL, typeNameMlcd, true);
    if (typeStrObj == NULL) {
        VmError(vm, RT_IME_BUILTIN_TYPE_STRRETURN);
        return MakeNil();
    }

    return MakeObject(typeStrObj);
}

static PValue builtinLen(PVm *vm, PValue *args, u64 argc) {
    PValue target = args[0];
    if (!IsValueObj(target)) {
        VmError(vm, RT_BUILTIN_LEN_INVALID_TYPE, ValueTypeToStr(target));
        return MakeNil();
    }
    PObj *targetObj = ValueAsObj(target);
    if (!ObjectHasLen(targetObj)) {
        VmError(vm, RT_BUILTIN_LEN_INVALID_TYPE, ValueTypeToStr(target));
        return MakeNil();
    }
    return MakeNumber(GetObjectLength(targetObj));
}

static PValue builtinAppend(PVm *vm, PValue *args, u64 argc) {
    PValue target = args[0];
    if (!IsValueObjType(target, OT_ARR)) {
        VmError(vm, RT_BUILTIN_APPEND_TARGET_NOT_ARRAY, ValueTypeToStr(target));
        return MakeNil();
    }
    PObj *obj = ValueAsObj(target);

    u64 elmCount = argc - 1;
    for (u64 i = 0; i < elmCount; i++) {
        PValue elm = args[i + 1];
        if (!ArrayObjPushValue(obj, elm)) {
            VmError(vm, RT_IME_BUILTIN_APPEND_PUSH_FAILED);
            return MakeNil();
        }
    }

    return MakeNumber((double)obj->v.OArray.count);
}

static PValue builtinError(PVm *vm, PValue *args, u64 argc) {
    PValue rawMsg = args[0];
    if (!IsValueObjType(rawMsg, OT_STR)) {
        VmError(vm, RT_BUILTIN_ERROR_INVALID_TYPE, ValueTypeToStr(rawMsg));
        return MakeNil();
    }
    struct OString *strObj = &ValueAsObj(rawMsg)->v.OString;

    VmError(vm, RT_TEMPLATE, strObj->value);
    return MakeNil();
}

static PValue builtinGetArgs(PVm *vm, PValue *args, u64 argc) {
    (void)args;
    (void)argc;

    int sargCount = vm->scriptArgCount;
    char **sargs = vm->scriptArgs;

    if (sargCount == 0 || sargs == NULL) {
        PObj *emptyArray = NewArrayObject(vm->gc, NULL, NULL, 0);

        if (emptyArray == NULL) {
            VmError(vm, RT_IME_BUILTIN_ARGS_ARRAY_CREATE_FAILED);
            return MakeNil();
        }

        return MakeObject(emptyArray);
    }

    PValue *items = NULL;
    for (int i = 0; i < sargCount; i++) {
        PObj *strObj = NewStrObject(vm->gc, NULL, sargs[i], false);
        if (strObj == NULL) {
            arrfree(items); // GC will handle orphaned objects, just free array
            VmError(vm, RT_IME_BUILTIN_ARGS_ARRAY_ITEM_CREATE_FAILED);
            return MakeNil();
        }

        arrpush(items, MakeObject(strObj));
    }

    PObj *arr = NewArrayObject(vm->gc, NULL, items, sargCount);
    if (arr == NULL) {
        arrfree(items); // GC will handle orphaned objects, just free array
        VmError(vm, RT_IME_BUILTIN_ARGS_ARRAY_CREATE_FAILED);
        return MakeNil();
    }

    return MakeObject(arr);
}

void RegisterBuiltins(PVm *vm) {
    if (vm == NULL) {
        return;
    }

    if (vm->gc == NULL) {
        return;
    }

    StdlibEntry builtinEntries[] = {
        MakeStdlibEntry(NAME_CLOCK_EN, builtinClock, 0),
        MakeStdlibEntry(NAME_CLOCK_BN, builtinClock, 0),
        MakeStdlibEntry(NAME_CLOCK_PN, builtinClock, 0),
        MakeStdlibEntry(NAME_SHOW_EN, builtinShow, -1),
        MakeStdlibEntry(NAME_SHOW_BN, builtinShow, -1),
        MakeStdlibEntry(NAME_SHOW_PN, builtinShow, -1),
        MakeStdlibEntry(NAME_TYPE_EN, builtinType, 1),
        MakeStdlibEntry(NAME_TYPE_BN, builtinType, 1),
        MakeStdlibEntry(NAME_TYPE_PN, builtinType, 1),
        MakeStdlibEntry(NAME_LEN_EN, builtinLen, 1),
        MakeStdlibEntry(NAME_LEN_BN, builtinLen, 1),
        MakeStdlibEntry(NAME_LEN_PN, builtinLen, 1),
        MakeStdlibEntry(NAME_APPEND_EN, builtinAppend, 2),
        MakeStdlibEntry(NAME_APPEND_BN, builtinAppend, 2),
        MakeStdlibEntry(NAME_APPEND_PN, builtinAppend, 2),
        MakeStdlibEntry(NAME_ERROR_EN, builtinError, 1),
        MakeStdlibEntry(NAME_ERROR_BN, builtinError, 1),
        MakeStdlibEntry(NAME_ERROR_PN, builtinError, 1),
        MakeStdlibEntry(NAME_ARGS_EN, builtinGetArgs, 0),
        MakeStdlibEntry(NAME_ARGS_BN, builtinGetArgs, 0),
        MakeStdlibEntry(NAME_ARGS_PN, builtinGetArgs, 0),
    };

    u64 count = ArrCount(builtinEntries);
    PushStdlibEntries(
        vm, vm->globals, BUILTIN_MODULE_NAME, builtinEntries, count
    );
}
