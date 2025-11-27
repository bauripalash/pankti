#include "../include/gc.h"
#include "../include/alloc.h"
#include "../include/env.h"
#include "../external/stb/stb_ds.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#if defined (DEBUG_GC)
#include "../include/ansicolors.h"
#endif

static void sweep(Pgc * gc);
static void markRoots(Pgc * gc);
static void markEnv(Pgc * gc, PEnv * env);
static void markValue(Pgc * gc, PValue value);
static void markObject(Pgc * gc, PObj * obj);
static void markObjectChildren(Pgc * gc, PObj * obj);


Pgc *NewGc(void) {
    Pgc *gc = PCreate(Pgc);
    gc->disable = false;
#if defined (STRESS_GC)
    gc->stress = true;
#else
    gc->stress = false;
#endif
    gc->objects = NULL;
    gc->stmts = NULL;
    gc->timestamp = (uint64_t)time(NULL);
	gc->rootEnvs = NULL;
	gc->rootEnvCount = 0;

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
    if (gc == NULL) {
        return;
    }

    freeStatements(gc);
    freeObjects(gc);

	if (gc->rootEnvs != NULL && gc->rootEnvCount > 0) {
		arrfree(gc->rootEnvs);
	}

    PFree(gc);
}


void RegisterRootEnv(Pgc * gc, PEnv * env){
	if (gc == NULL || env == NULL) {
		return;
	}

	arrpush(gc->rootEnvs, env);
	gc->rootEnvCount++;
}


void UnregisterRootEnv(Pgc * gc, PEnv * env){
	if (gc == NULL || env == NULL) {
		return;
	}
	for (size_t i = 0; i < gc->rootEnvCount; i++) {
		if (gc->rootEnvs[i] == env) {
			//With this delswap we don't to move items
			arrdelswap(gc->rootEnvs, i); 			
			break;
		}
	}
	gc->rootEnvCount--;
}
void GcCounterNew(Pgc * gc){
	if (gc == NULL) {
		return;
	}

	gc->objCount++;
	if (gc->objCount > gc->nextGc) {
		gc->needCollect = true;
	}
}
void GcCounterFree(Pgc * gc){
	if (gc == NULL) {
		return;
	}
	if (gc->objCount > 0) {
		gc->objCount--;
	}
}

void GcUpdateThreshold(Pgc *gc){
	if (gc == NULL) {
		return;
	}

	size_t newThreshold = gc->objCount * GC_GROW_FACTOR;
	if (newThreshold < GC_OBJ_THRESHOLD) {
		newThreshold = GC_OBJ_THRESHOLD;
	}

	gc->nextGc = newThreshold;
}

void CollectGarbage(Pgc * gc){
	if (gc == NULL) {
		return;
	}

	if (gc->disable) {
		return;
	}

	if (gc->stress || gc->needCollect) {
#if defined (DEBUG_GC)
		printf(TERMC_YELLOW "[DEBUG] [GC] Starting Garbage Collection\n" TERMC_RESET);
		printf(TERMC_BYELLOW "[DEBUG] [GC] [Object Count : %zu]\n" TERMC_RESET, gc->objCount);
#endif
		markRoots(gc);
		sweep(gc);
		GcUpdateThreshold(gc);
		gc->needCollect = false;
#if defined (DEBUG_GC)
		printf(TERMC_YELLOW "[DEBUG] [GC] Finished Garbage Collection\n" TERMC_RESET);
		printf(TERMC_BYELLOW "[DEBUG] [GC] [Object Count : %zu]\n" TERMC_RESET, gc->objCount);
#endif
		
	}

}

static void markRoots(Pgc * gc){
	if (gc == NULL) {
		return;
	}

	if (gc->rootEnvs != NULL && gc->rootEnvCount > 0) {
		for (size_t i = 0; i < gc->rootEnvCount; i++) {
			markEnv(gc, gc->rootEnvs[i]);
		}
	}
}

static void sweep(Pgc * gc){
	if (gc == NULL) {
		return;
	}
	PObj * prev = NULL;
	PObj * obj = gc->objects;
	while (obj != NULL) {
		PObj * next = obj->next;
		if (obj->marked) {
			obj->marked = false;
			prev = obj;
		}else{
			if (prev != NULL) {
				prev->next = next;
			} else {
				gc->objects = next;
			}

			FreeObject(gc, obj);
		}

		obj = next;
	}
}

static void markValue(Pgc * gc, PValue value){
	if (IsValueObj(value)) {
		markObject(gc, ValueAsObj(value));
	}
}


static void markObject(Pgc * gc, PObj * obj){
	if (gc == NULL) {
		return;
	}

	if (obj == NULL) {
		return;
	}

	if (obj->marked) {
		return;
	}

	obj->marked = true;
	markObjectChildren(gc, obj);
}

static void markEnv(Pgc * gc, PEnv * env){
	if (gc == NULL) {
		return;
	}

	if (env == NULL) {
		return;
	}

	if (env->table == NULL) {
		return;
	}

	size_t envCount = (size_t)hmlen(env->table);

	for (size_t i = 0; i < envCount; i++) {
		markValue(gc, env->table[i].value);
	}
}



static void markObjectChildren(Pgc * gc, PObj * obj){
	if (gc == NULL) {
		return;
	}

	if (obj == NULL) {
		return;
	}

	switch (obj->type) {
		case OT_NATIVE:
		case OT_ERROR:
		case OT_STR: {
			break;
		}

		case OT_ARR:{
			struct OArray * arr = &obj->v.OArray;
			for (size_t i = 0; i < arr->count; i++) {
				markValue(gc, arr->items[i]);
			}
			break;
		}

		case OT_MAP:{
			struct OMap * map = &obj->v.OMap;
			size_t count = map->count;
			for (size_t i = 0; i < count; i++) {
				markValue(gc, map->table[i].vkey);
				markValue(gc, map->table[i].value);
			}
			break;
			
		}
		case OT_FNC:{
			struct OFunction * func = &obj->v.OFunction;
			if (func->body) {
				//MarkStatements
			}

			if (func->env != NULL) {
				markEnv(gc, func->env);
			}
			
			break;

		}

		case OT_UPVAL:{
			markValue(gc, obj->v.OUpval.value);
			break;
		}

	}
}
