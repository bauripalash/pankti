#include "include/native.h"
#include "external/xxhash/xxhash.h"
#include "include/env.h"
#include "include/gc.h"
#include "include/interpreter.h"
#include "include/object.h"
#include "include/utils.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

static PValue ntvClock(PInterpreter *it, PValue *args, size_t argc) {
    clock_t c = clock();
    double sec = (double)c / (double)CLOCKS_PER_SEC;
    return MakeNumber(sec);
}

static PValue ntvShow(PInterpreter *it, PValue *args, size_t argc) {
    for (size_t i = 0; i < argc; i++) {
        PrintValue(&args[i]);
        if (i + 1 < argc) {
            printf(" ");
        }
    }
    printf("\n");
    return MakeNil();
}

static PValue ntvLen(PInterpreter *it, PValue * args, size_t argc){
	PValue *target = &args[0];
	if (target->type != VT_OBJ) {
		return MakeError(it->gc, "Length can not be calculated for this value");
	}
	PObj * targetObj = target->v.obj;
	if (!ObjectHasLen(targetObj)) {
		return MakeError(it->gc, "Length can not be calculated for this value");
	}
	return MakeNumber(GetObjectLength(targetObj));
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
}
