#include "../include/alloc.h"
#include "../include/ansicolors.h"
#include "../include/env.h"
#include "../include/gc.h"
#include "../include/object.h"
#include <stdio.h>

PObj *NewObject(Pgc *gc, PObjType type) {
    PObj *o = PCreate(PObj);
    o->type = type;
    o->next = gc->objects;
    gc->objects = o;
#if defined DEBUG_GC
    printf(
        TERMC_BLUE "[DEBUG] [GC] %p New Object : %s\n" TERMC_RESET, o,
        ObjTypeToString(type)
    );
#endif
    return o;
}
PObj *NewNumberObj(Pgc *gc, double value) {
    PObj *o = NewObject(gc, OT_NUM);
    o->v.num = value;
    return o;
}

PObj *NewBoolObj(Pgc *gc, bool value) {
    if (value) {
        return gc->onlyTrueObj;
    } else {
        return gc->onlyFalseObj;
    }
}

PObj *NewStrObject(Pgc *gc, char *value) {
    PObj *o = NewObject(gc, OT_STR);
    o->v.str = value;
    return o;
}

PObj *NewNilObject(Pgc *gc) { return gc->onlyNilObj; }

PObj *NewReturnObject(Pgc *gc, PObj *value) {
    PObj *o = NewObject(gc, OT_RET);
    o->v.OReturn.rvalue = value;
    return o;
}

PObj *NewBreakObject(Pgc *gc) {
    PObj *o = NewObject(gc, OT_BRK);
    return o;
}

PObj *NewFuncObject(
    Pgc *gc, Token *name, Token **params, PStmt *body, void *env, int count
) {
    PObj *o = NewObject(gc, OT_FNC);
    o->v.OFunction.name = name;
    o->v.OFunction.params = params;
    o->v.OFunction.body = body;
    o->v.OFunction.env = env;
    o->v.OFunction.paramCount = count;
    return o;
}

static inline void freeBaseObj(PObj *o) {
    if (o != NULL) {
        free(o);
        o = NULL;
    }
}

void FreeObject(Pgc *gc, PObj *o) {
    if (o == NULL) {
        return;
    }

#if defined DEBUG_GC
    printf(
        TERMC_GREEN "[DEBUG] [GC] Freeing Object : %p : %s : " TERMC_RESET, o,
        "s"
    );
    if (o != NULL) {

        // PrintObject(o);
    }
    printf("\n");
#endif

    switch (o->type) {
        case OT_FNC: {
            struct OFunction *f = &o->v.OFunction;
            FreeEnv(f->env);
            freeBaseObj(o);
            break;
        }
        case OT_RET: {
            // FreeObject(gc, o->v.OReturn.rvalue);
            freeBaseObj(o);
            break;
        }
        case OT_STR: {
            // where does the string come from?
            freeBaseObj(o);
            break;
        }
        case OT_BRK:
        case OT_NIL:
        case OT_BOOL:
        case OT_NUM: {
            freeBaseObj(o);
            break;
        }
    }
}
