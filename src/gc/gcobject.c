#include "../external/stb/stb_ds.h"
#include "../include/alloc.h"
#include "../include/ansicolors.h"
#include "../include/env.h"
#include "../include/gc.h"
#include "../include/object.h"
#include <stdio.h>
#include <string.h>
#include <stddef.h>

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

PObj *NewStrObject(Pgc *gc, char *value) {
    PObj *o = NewObject(gc, OT_STR);
    if (value == NULL) {
    	o->v.str = value;
		return o;
    }

	size_t slen = strlen(value);
	char * str = PCalloc(slen + 1, sizeof(char));
	if (str == NULL) {
		o->v.str = NULL;
		return o;
	}

	size_t rdi = 0;
	for (size_t i = 0; i < slen; i++) {
		char c = value[i];
		if (c == '\\' && i + 1 < slen) {
			i++;
			char ec = value[i];
			switch (ec) {
				case '\\': str[rdi++] = '\\';break;
				case 'a': str[rdi++] = '\a';break;
				case 'b': str[rdi++] = '\b';break;
				case 'f': str[rdi++] = '\f';break;
				case 'n': str[rdi++] = '\n';break;
				case 'r': str[rdi++] = '\r';break;
				case 't': str[rdi++] = '\t';break;
				case 'v': str[rdi++] = '\v';break;
				default: str[rdi++] = ec; break;
			}
		
		}else{
			str[rdi] = c;
			rdi++;
		}
	}

	if (rdi <= slen) {
		str[rdi] = '\0';
	}else{
		str[slen] = '\0'; // do we need this?
	}

	o->v.str = str;

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
			PFree(o->v.str);
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
