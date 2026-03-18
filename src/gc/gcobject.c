#include "../external/stb/stb_ds.h"
#include "../include/alloc.h"
#include "../include/env.h"
#include "../include/gc.h"
#include "../include/object.h"
#include "../include/opcode.h"
#include "../include/utils.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#if defined DEBUG_GC
#include "../include/printer.h"
#include "../include/terminal.h"
#endif

#define GcPopObj(gc, o)                                                        \
    do {                                                                       \
        (gc)->objects = (o)->next;                                             \
        FreeObject((gc), (o));                                                 \
    } while (0)

PObj *NewObject(Pgc *gc, PObjType type) {
    PObj *o = PCreate(PObj);
    if (o == NULL) {
        return NULL;
    }
    o->type = type;
    o->next = gc->objects;
    gc->objects = o;
    o->marked = false;
#if defined DEBUG_GC
    PanPrint(
        "%s[DEBUG] [GC] %p New Object : %s%s\n", TermBlue(), (void *)o,
        ObjTypeToString(type), TermReset()
    );
#endif
    GcCounterNew(gc);
    return o;
}

PObj *NewStrObject(Pgc *gc, Token *name, char *value, bool virt) {
    PObj *o = NewObject(gc, OT_STR);
    if (o == NULL) {
        return NULL;
    }
    o->v.OString.name = name;
    o->v.OString.value = value;
    o->v.OString.isVirtual = virt;
    u64 hash = StrHash(value, StrLength(value), gc->timestamp);
    o->v.OString.hash = hash;
    return o;
}

PObj *NewFuncObject(
    Pgc *gc, Token *name, Token **params, PStmt *body, void *env, u64 count
) {
    PObj *o = NewObject(gc, OT_FNC);
    if (o == NULL) {
        return NULL;
    }
    o->v.OFunction.name = name;
    o->v.OFunction.params = params;
    o->v.OFunction.body = body;
    o->v.OFunction.env = env;
    o->v.OFunction.paramCount = count;
    return o;
}

PObj *NewComFuncObject(Pgc *gc, Token *name) {
    PObj *o = NewObject(gc, OT_COMFNC);
    if (o == NULL) {
        return NULL;
    }
    o->v.OComFunction.rawName = name;

    PBytecode *btCode = NewBytecode();
    if (btCode == NULL) {
        GcPopObj(gc, o);
        return NULL;
    }

    o->v.OComFunction.code = btCode;
    o->v.OComFunction.paramCount = 0;
    o->v.OComFunction.strName = NULL;

    if (name != NULL) {
        PObj *strName = NewStrObject(gc, name, name->lexeme, false);
        if (strName == NULL) {
            GcPopObj(gc, o);
            return NULL;
        }
        o->v.OComFunction.strName = strName;
    }
    return o;
}

PObj *NewArrayObject(Pgc *gc, Token *op, PValue *items, u64 count) {
    PObj *o = NewObject(gc, OT_ARR);
    if (o == NULL) {
        return NULL;
    }
    o->v.OArray.items = items;
    o->v.OArray.count = count;
    o->v.OArray.op = op;
    return o;
}

PObj *NewMapObject(Pgc *gc, Token *op) {
    PObj *o = NewObject(gc, OT_MAP);
    if (o == NULL) {
        return NULL;
    }
    o->v.OMap.table = NULL;
    o->v.OMap.op = op;
    o->v.OMap.count = 0;
    return o;
}

PObj *NewNativeFnObject(Pgc *gc, const char *name, NativeFn fn, int arity) {
    PObj *o = NewObject(gc, OT_NATIVE);
    if (o == NULL) {
        return NULL;
    }
    char *nameStr = NULL;

    if (name != NULL) {
        nameStr = StrDuplicate(name, StrLength(name));
        if (nameStr == NULL) {
            GcPopObj(gc, o);
            return NULL;
        }
    }

    o->v.ONative.name = nameStr;
    o->v.ONative.fn = fn;
    o->v.ONative.arity = arity;
    return o;
}

PObj *NewModuleObject(Pgc *gc, char *name, char *path) {
    PObj *o = NewObject(gc, OT_MODULE);
    if (o == NULL) {
        return NULL;
    }
    o->v.OModule.nameHash = 0;
    o->v.OModule.customName = NULL;
    o->v.OModule.path = NULL;

    if (name != NULL) {
        char *customName = StrDuplicate(name, StrLength(name));
        if (customName == NULL) {
            GcPopObj(gc, o);
            return NULL;
        }

        o->v.OModule.customName = customName;
        o->v.OModule.nameHash = StrHash(name, StrLength(name), gc->timestamp);
    }

    if (path != NULL) {
        char *pathStr = StrDuplicate(path, StrLength(path));
        if (pathStr == NULL) {
            GcPopObj(gc, o);
            return NULL;
        }

        o->v.OModule.path = pathStr;
    }

    return o;
}

PObj *NewUpvalueObject(Pgc *gc, PValue initValue) {
    PObj *o = NewObject(gc, OT_UPVAL);
    if (o == NULL) {
        return NULL;
    }
    o->v.OUpval.value = initValue;
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
    PanPrint(
        "%s[DEBUG] [GC] Freeing Object : %p : %s : %s", TermGreen(), (void *)o,
        ObjTypeToString(o->type), TermReset()
    );
    if (o->type == OT_UPVAL) {
        PanPrint("<UpValue> : %ld", (u64)o);
    } else {
        PrintObject(o);
    }
    PanPrint("\n");
#endif

    switch (o->type) {
        case OT_FNC: {
            struct OFunction *f = &o->v.OFunction;
            ReallyFreeEnv(f->env);
            freeBaseObj(o);
            break;
        }
        case OT_COMFNC: {
            struct OComFunction *f = &o->v.OComFunction;
            FreeBytecode(f->code);
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
            struct ONative *nativeFn = &o->v.ONative;
            if (nativeFn->name != NULL) {
                PFree(nativeFn->name);
            }
            freeBaseObj(o);
            break;
        }
        case OT_MODULE: {
            if (o->v.OModule.customName != NULL) {
                PFree(o->v.OModule.customName);
            }
            if (o->v.OModule.path != NULL) {
                PFree(o->v.OModule.path);
            }
            freeBaseObj(o);
            break;
        }

        case OT_UPVAL: {
            freeBaseObj(o);
            break;
        }
    }

    GcCounterFree(gc);
}
