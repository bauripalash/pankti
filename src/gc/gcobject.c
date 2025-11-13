#include "../external/stb/stb_ds.h"
#include "../include/alloc.h"
#include "../include/ansicolors.h"
#include "../include/env.h"
#include "../include/gc.h"
#include "../include/object.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

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

PObj *NewStrObject(Pgc *gc, Token *name, char *value, bool virt) {
    PObj *o = NewObject(gc, OT_STR);
    o->v.OString.name = name;
    o->v.OString.value = value;
    o->v.OString.isVirtual = virt;
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

PObj *NewArrayObject(Pgc *gc, Token *op, PValue *items, int count) {
    PObj *o = NewObject(gc, OT_ARR);

    o->v.OArray.items = items;
    o->v.OArray.count = count;
    o->v.OArray.op = op;
    return o;
}

PObj *NewMapObject(Pgc *gc, Token *op) {
    PObj *o = NewObject(gc, OT_MAP);
    o->v.OMap.table = NULL;
    o->v.OMap.op = op;
    o->v.OMap.count = 0;
    return o;
}

PObj *NewNativeFnObject(Pgc *gc, Token *name, NativeFn fn, int arity) {
    PObj *o = NewObject(gc, OT_NATIVE);
    o->v.ONative.name = name;
    o->v.ONative.fn = fn;
    o->v.ONative.arity = arity;
    return o;
}

static inline void freeBaseObj(PObj *o) {
    if (o != NULL) {
        PFree(o);
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

        case OT_STR: {
            struct OString *s = &o->v.OString;
            if (s->isVirtual) {
                PFree(s->value);
            }
            freeBaseObj(o);
            break;
        }

        case OT_ARR: {
            struct OArray *arr = &o->v.OArray;
            arrfree(arr->items);
            freeBaseObj(o);
            break;
        }
        case OT_MAP: {
            struct OMap *map = &o->v.OMap;
            if (map->table != NULL) {
                hmfree(map->table);
            }
            freeBaseObj(o);
            break;
        }
        case OT_NATIVE: {
            freeBaseObj(o);
        }
    }
}
