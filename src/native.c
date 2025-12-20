#include "include/native.h"
#include "include/env.h"
#include "include/gc.h"
#include "include/interpreter.h"
#include "include/object.h"
#include "include/printer.h"
#include "include/ptypes.h"
#include "include/utils.h"
#include <stddef.h>
#include <stdint.h>
#include <time.h>

static PValue ntvClock(PInterpreter *it, PValue *args, u64 argc) {
    clock_t c = clock();
    double sec = (double)c / (double)CLOCKS_PER_SEC;
    return MakeNumber(sec);
}

static PValue ntvShow(PInterpreter *it, PValue *args, u64 argc) {
    for (u64 i = 0; i < argc; i++) {
        PrintValue(args[i]);
        if (i + 1 < argc) {
            PanPrint(" ");
        }
    }
    return MakeNil();
}

static PValue ntvLen(PInterpreter *it, PValue *args, u64 argc) {
    PValue target = args[0];
    if (!IsValueObj(target)) {
        return MakeError(it->gc, "Length can not be calculated for this value");
    }
    PObj *targetObj = ValueAsObj(target);
    if (!ObjectHasLen(targetObj)) {
        return MakeError(it->gc, "Length can not be calculated for this value");
    }
    return MakeNumber(GetObjectLength(targetObj));
}

static PValue ntvPushArray(PInterpreter *it, PValue *args, u64 argc) {

    if (argc < 2) {
        return MakeError(
            it->gc,
            "Push needs atleast two arguments, array and an element to add"
        );
    }

    PValue rawArray = args[0];

    if (!IsValueObjType(rawArray, OT_ARR)) {
        return MakeError(it->gc, "Push can only be used on arrays");
    }

    PObj *obj = ValueAsObj(rawArray);

    u64 elmCount = argc - 1;
    for (u64 i = 0; i < elmCount; i++) {
        PValue elm = args[i + 1];
        if (!ArrayObjPushValue(obj, elm)) {
            return MakeError(
                it->gc, "Interenal Error : Failed to push item to array"
            );
        }
    }

    return MakeNumber(obj->v.OArray.count);
}

void RegisterNatives(PInterpreter *it, PEnv *env) {
    if (it == NULL) {
        return;
    }

    if (it->gc == NULL) {
        return;
    }

    if (env == NULL) {
        return;
    }

    PObj *clockFn = NewNativeFnObject(it->gc, NULL, ntvClock, 0);
    EnvPutValue(
        env, StrHash("clock", 5, it->gc->timestamp), MakeObject(clockFn)
    );
    PObj *showFn = NewNativeFnObject(it->gc, NULL, ntvShow, -1);
    EnvPutValue(env, StrHash("show", 4, it->gc->timestamp), MakeObject(showFn));

    PObj *lenFn = NewNativeFnObject(it->gc, NULL, ntvLen, 1);
    EnvPutValue(env, StrHash("len", 3, it->gc->timestamp), MakeObject(lenFn));

    PObj *pushFn = NewNativeFnObject(it->gc, NULL, ntvPushArray, -1);
    EnvPutValue(
        env, StrHash("append", 6, it->gc->timestamp), MakeObject(pushFn)
    );
}
