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

#define NAME_CLOCK_EN  "clock"
#define NAME_CLOCK_BN  "সময়"
#define NAME_CLOCK_PN "somoy"

#define NAME_SHOW_EN   "show"
#define NAME_SHOW_BN   "দেখাও"
#define NAME_SHOW_PN   "dekhao"

#define NAME_LEN_EN    "len"
#define NAME_LEN_BN    "আয়তন"
#define NAME_LEN_PN    "ayoton"

#define NAME_APPEND_EN "append"
#define NAME_APPEND_BN "সংযোগ"
#define NAME_APPEND_PN "songjog"

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

static PValue ntvAppend(PInterpreter *it, PValue *args, u64 argc) {

    if (argc < 2) {
        return MakeError(
            it->gc, "append(...) needs atleast two arguments, for appending to array"
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
                    it->gc, "Interenal Error : Failed to append item to array"
                );
            }
        }

        return MakeNumber((double)obj->v.OArray.count);
    } else if (IsValueObjType(target, OT_MAP)) {
        if (argc != 3) {
            return MakeError(
                it->gc,
                "append(...) needs atleast 3 arguments, map and a key-value pair"
            );
        }

        PObj *obj = ValueAsObj(target);
        PValue key = args[1];
        PValue value = args[2];

        if (!MapObjPushPair(obj, key, value, it->gc->timestamp)) {
            return MakeError(
                it->gc, "Interenal Error : Failed to append key-value pair to map"
            );
        }

        return MakeNumber((double)obj->v.OMap.count);
    }

    return MakeError(
        it->gc, "append(...) function only works on arrays and maps"
    );
}

#define DefStrHash(s, it) ((StrHash(s, DefStrLen(s), it->gc->timestamp)))

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
    PObj *showFn = NewNativeFnObject(it->gc, NULL, ntvShow, -1);
    PObj *lenFn = NewNativeFnObject(it->gc, NULL, ntvLen, 1);
    PObj *appendFn = NewNativeFnObject(it->gc, NULL, ntvAppend, -1);

    EnvPutValue(env, DefStrHash(NAME_CLOCK_EN, it), MakeObject(clockFn));
    EnvPutValue(env, DefStrHash(NAME_CLOCK_BN, it), MakeObject(clockFn));
    EnvPutValue(env, DefStrHash(NAME_CLOCK_PN, it), MakeObject(clockFn));

    EnvPutValue(env, DefStrHash(NAME_SHOW_EN, it), MakeObject(showFn));
    EnvPutValue(env, DefStrHash(NAME_SHOW_BN, it), MakeObject(showFn));
    EnvPutValue(env, DefStrHash(NAME_SHOW_PN, it), MakeObject(showFn));

    EnvPutValue(env, DefStrHash(NAME_LEN_EN, it), MakeObject(lenFn));
    EnvPutValue(env, DefStrHash(NAME_LEN_BN, it), MakeObject(lenFn));
    EnvPutValue(env, DefStrHash(NAME_LEN_PN, it), MakeObject(lenFn));

    EnvPutValue(env, DefStrHash(NAME_APPEND_EN, it), MakeObject(appendFn));
    EnvPutValue(env, DefStrHash(NAME_APPEND_BN, it), MakeObject(appendFn));
    EnvPutValue(env, DefStrHash(NAME_APPEND_PN, it), MakeObject(appendFn));
}
