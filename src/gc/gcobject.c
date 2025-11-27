#include "../external/stb/stb_ds.h"
#include "../include/alloc.h"
#include "../include/env.h"
#include "../include/gc.h"
#include "../include/object.h"
#include "../include/utils.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if defined DEBUG_GC
#include "../include/ansicolors.h"
#endif

PObj *NewObject(Pgc *gc, PObjType type) {
    PObj *o = PCreate(PObj);
    o->type = type;
    o->next = gc->objects;
    gc->objects = o;
	o->marked = false;
#if defined DEBUG_GC
    printf(
        TERMC_BLUE "[DEBUG] [GC] %p New Object : %s\n" TERMC_RESET, (void*)o,
        ObjTypeToString(type)
    );
#endif
	GcCounterNew(gc);
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
    Pgc *gc, Token *name, Token **params, PStmt *body, void *env, size_t count
) {
    PObj *o = NewObject(gc, OT_FNC);
    o->v.OFunction.name = name;
    o->v.OFunction.params = params;
    o->v.OFunction.body = body;
    o->v.OFunction.env = env;
    o->v.OFunction.paramCount = count;
    return o;
}

PObj *NewArrayObject(Pgc *gc, Token *op, PValue *items, size_t count) {
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

PObj *NewErrorObject(Pgc *gc, char *msg) {
    PObj *o = NewObject(gc, OT_ERROR);
    if (msg != NULL) {
        o->v.OError.msg = StrDuplicate(msg, strlen(msg));
    }
    return o;
}


PObj * NewUpvalueObject(Pgc * gc, PValue initValue){
	PObj * o = NewObject(gc, OT_UPVAL);
	o->v.OUpval.value = initValue;
	return o;
}

PValue MakeError(Pgc *gc, char *msg) {
    PObj *o = NewErrorObject(gc, msg);
    return MakeObject(o);
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
        TERMC_GREEN "[DEBUG] [GC] Freeing Object : %p : %s : " TERMC_RESET, 
		(void*)o, 
		ObjTypeToString(o->type)
    );
	if (o->type == OT_UPVAL) {
		printf("<UpValue> : %ld", (uint64_t)o);
	}else{
		PrintObject(o);
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
			arr->items = NULL;
            freeBaseObj(o);
            break;
        }
        case OT_MAP: {
            struct OMap *map = &o->v.OMap;
            if (map->table != NULL) {
                hmfree(map->table);
            }
			map->table = NULL;
            freeBaseObj(o);
            break;
        }
        case OT_NATIVE: {
            freeBaseObj(o);
            break;
        }

        case OT_ERROR: {
            if (o->v.OError.msg != NULL) {
                PFree(o->v.OError.msg);
            }
            freeBaseObj(o);
            break;
        }

		case OT_UPVAL:{
			freeBaseObj(o);
			break;
		}
    }

	GcCounterFree(gc);
}
