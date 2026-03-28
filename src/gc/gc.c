#include "../include/gc.h"
#include "../external/stb/stb_ds.h"
#include "../include/alloc.h"
#include "../include/core.h"
#include "../include/env.h"
#include "../include/flags.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

#if defined(PANKTI_BUILD_DEBUG)
#include "../include/printer.h"
#include "../include/terminal.h"
#endif

static void sweep(Pgc *gc);
static void markRoots(Pgc *gc);
static void darkenObject(Pgc *gc, PObj *obj);
static void traceRefs(Pgc *gc);

Pgc *NewGc(void) {
    Pgc *gc = PCreate(Pgc);
    if (gc == NULL) {
        return NULL;
    }
    gc->strings = NewStringPool();
    if (gc->strings == NULL) {
        PFree(gc);
        return NULL;
    }
    gc->disable = false;
    gc->needCollect = false;
    gc->nextGc = GC_OBJ_THRESHOLD;
#if defined(PANKTI_BUILD_DEBUG)
    gc->stress = FLAG_STRESS_GC;
#else
    gc->stress = false;
#endif
    gc->objects = NULL;
    gc->stmts = NULL;
    gc->timestamp = (u64)time(NULL);
    gc->objCount = 0;
    gc->grayStack = NULL;
    gc->grayStackCount = 0;
    arrsetcap(gc->grayStack, 16);
    return gc;
}

static void freeObjects(Pgc *gc) {
    PObj *obj = gc->objects;
    while (obj != NULL) {
        PObj *nextObj = obj->next;
        FreeObject(gc, obj);
        obj = nextObj;
    }
}

static void freeStatements(Pgc *gc) {
    PStmt *stmt = gc->stmts;
    while (stmt != NULL) {
        PStmt *nextStmt = stmt->next;
        FreeStmt(gc, stmt);
        stmt = nextStmt;
    }
}

void FreeGc(Pgc *gc) {
#if defined(PANKTI_BUILD_DEBUG)
    if (FLAG_DEBUG_GC) {
        PanPrint(
            "%s[DEBUG] [GC] Shutting Down%s : [%llu]\n", TermYellow(),
            TermReset(), (unsigned long long)gc->objCount
        );
    }
#endif
    if (gc == NULL) {
        return;
    }

    freeStatements(gc);
    freeObjects(gc);
    if (gc->strings != NULL) {
        FreeStringPool(gc->strings);
        gc->strings = NULL;
    }

    if (gc->grayStack != NULL) {
        arrfree(gc->grayStack);
    }

    PFree(gc);
#if defined(PANKTI_BUILD_DEBUG)
    if (FLAG_DEBUG_GC) {
        PanPrint(
            "%s[DEBUG] [GC] Finished Shutting Down%s\n", TermYellow(),
            TermReset()
        );
    }
#endif
}
void GcCounterNew(Pgc *gc) {
    if (gc == NULL) {
        return;
    }

    gc->objCount++;
    if (gc->objCount > gc->nextGc) {
        gc->needCollect = true;
    }
}
void GcCounterFree(Pgc *gc) {
    if (gc == NULL) {
        return;
    }
    if (gc->objCount > 0) {
        gc->objCount--;
    }
}

void GcUpdateThreshold(Pgc *gc) {
    if (gc == NULL) {
        return;
    }

    u64 newThreshold = gc->objCount * GC_GROW_FACTOR;
    if (newThreshold < GC_OBJ_THRESHOLD) {
        newThreshold = GC_OBJ_THRESHOLD;
    }

    gc->nextGc = newThreshold;
}

void CollectGarbage(Pgc *gc) {
    if (gc == NULL) {
        return;
    }

    if (gc->disable) {
        return;
    }

    if (gc->stress || gc->needCollect) {

#if defined(PANKTI_BUILD_DEBUG)
        if (FLAG_DEBUG_GC) {
            PanPrint(
                "%s[DEBUG] [GC] Starting Garbage Collection%s : [%llu] : "
                "[T:%llu]\n",
                TermYellow(), TermReset(), (unsigned long long)gc->objCount,
                (unsigned long long)gc->nextGc
            );

            PanPrint(
                "    %s[DEBUG] [GC] Starting GC Marking%s\n", TermPurple(),
                TermReset()
            );
        }

#endif

        markRoots(gc);
        traceRefs(gc);
        StringPoolRemoveUnmarked(gc->strings);

#if defined(PANKTI_BUILD_DEBUG)
        if (FLAG_DEBUG_GC) {
            PanPrint(
                "    %s[DEBUG] [GC] Finished GC Marking%s\n", TermPurple(),
                TermReset()
            );

            PanPrint(
                "    %s[DEBUG] [GC] Starting GC Sweeping%s\n", TermPurple(),
                TermReset()
            );
        }
#endif

        sweep(gc);

#if defined(PANKTI_BUILD_DEBUG)
        if (FLAG_DEBUG_GC) {
            PanPrint(
                "    %s[DEBUG] [GC] Finished GC Sweeping%s\n", TermPurple(),
                TermReset()
            );
            PanPrint(
                "%s[DEBUG] [GC] Finished Garbage Collection%s : [%llu]\n",
                TermYellow(), TermReset(), (unsigned long long)gc->objCount
            );
        }
#endif

        GcUpdateThreshold(gc);
        gc->needCollect = false;
    }
}

static void traceRefs(Pgc *gc) {
    gc->grayStackCount = arrlen(gc->grayStack);
    if (gc->grayStackCount > 0) {
        while (arrlen(gc->grayStack) > 0) {
            PObj *obj = arrpop(gc->grayStack);
            darkenObject(gc, obj);
        }

        arrfree(gc->grayStack);
        gc->grayStackCount = 0;
        arrsetcap(gc->grayStack, 16);
    }
}

static void markVm(Pgc *gc) {
    if (gc != NULL && gc->core != NULL && gc->core->vm != NULL) {
        PVm *vm = gc->core->vm;
        MarkVmStack(vm);
        MarkVmFrames(vm);
        MarkVmOpenUpvals(vm);
        MarkSymbolTable(gc, vm->globals);
        MarkVmModules(vm);
    }
}

static void markRoots(Pgc *gc) {
    if (gc == NULL) {
        return;
    }

    markVm(gc);
    MarkCompilerRoots(gc->core->compiler);
}

static void sweep(Pgc *gc) {
    if (gc == NULL) {
        return;
    }
    PObj *prev = NULL;
    PObj *obj = gc->objects;
    while (obj != NULL) {
        if (obj->marked) {
            obj->marked = false;
            prev = obj;
            obj = obj->next;
        } else {
            PObj *unreached = obj;
            obj = obj->next;
            if (prev != NULL) {
                prev->next = obj;
            } else {
                gc->objects = obj;
            }

            FreeObject(gc, unreached);
        }
    }
}

void GcMarkObject(Pgc *gc, PObj *obj) {
    if (gc == NULL) {
        return;
    }

    if (obj == NULL) {
        return;
    }

#if defined(PANKTI_BUILD_DEBUG)

    if (FLAG_DEBUG_GC) {
        PanPrint(
            "        %s[DEBUG] [GC] Marking%s : ", TermPurple(), TermReset()
        );
        PrintObject(obj);
        PanPrint("\n");
    }

#endif

    if (obj->marked) {
        return;
    }

    obj->marked = true;
    arrput(gc->grayStack, obj);
}

void GcMarkValue(Pgc *gc, PValue value) {
    if (IsValueObj(value)) {
        GcMarkObject(gc, ValueAsObj(value));
    }
}

static void darkenObject(Pgc *gc, PObj *obj) {
    if (gc == NULL) {
        return;
    }

    if (obj == NULL) {
        return;
    }

#if defined(PANKTI_BUILD_DEBUG)
    if (FLAG_DEBUG_GC) {
        PanPrint(
            "        %s[DEBUG] [GC] Darkening%s : ", TermPurple(), TermReset()
        );
        PrintObject(obj);
        PanPrint("\n");
    }

#endif

    switch (obj->type) {
        case OT_NATIVE:
        case OT_STR:
        case OT_MODULE: {
            break;
        }

        case OT_CLOSURE: {
            struct OClosure *cls = &obj->v.OClosure;
            GcMarkObject(gc, cls->function);
            for (i16 i = 0; i < cls->upvalCount; i++) {
                GcMarkObject(gc, cls->upvals[i]);
            }
            break;
        }
        case OT_ARR: {
            struct OArray *arr = &obj->v.OArray;
            for (u64 i = 0; i < arr->count; i++) {
                GcMarkValue(gc, arr->items[i]);
            }
            break;
        }

        case OT_MAP: {
            struct OMap *map = &obj->v.OMap;
            u64 count = map->count;
            for (u64 i = 0; i < count; i++) {
                GcMarkValue(gc, map->table[i].vkey);
                GcMarkValue(gc, map->table[i].value);
            }
            break;
        }
        case OT_COMFNC: {
            struct OComFunction *func = &obj->v.OComFunction;
            if (func->strName != NULL) {
                GcMarkObject(gc, func->strName);
            }
            for (u64 i = 0; i < func->code->constCount; i++) {
                GcMarkValue(gc, func->code->constPool[i]);
            }

            break;
        }

        case OT_UPVAL: {
            GcMarkValue(gc, *obj->v.OUpval.location);
            break;
        }
    }
}
