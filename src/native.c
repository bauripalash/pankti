#include "include/native.h"
#include "include/env.h"
#include "include/gc.h"
#include "include/interpreter.h"
#include "include/object.h"
#include "include/utils.h"
#include <stdio.h>
#include <time.h>

static PValue ntvClock(PInterpreter *it, PValue *args, int argc) {
    clock_t c = clock();
    double sec = (double)c / (double)CLOCKS_PER_SEC;
    return MakeNumber(sec);
}

static PValue ntvShow(PInterpreter *it, PValue *args, int argc) {
    for (int i = 0; i < argc; i++) {
        PrintValue(&args[i]);
        if (i + 1 < argc) {
            printf(" ");
        }
    }
    printf("\n");
    return MakeNil();
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
    EnvPutValue(env, Fnv1a("clock", 5), MakeObject(clockFn));
    PObj *showFn = NewNativeFnObject(it->gc, NULL, ntvShow, -1);
    EnvPutValue(env, Fnv1a("show", 4), MakeObject(showFn));
}
