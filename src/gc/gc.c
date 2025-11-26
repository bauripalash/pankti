#include "../include/gc.h"
#include "../include/alloc.h"
#include "../include/env.h"
#include "../external/stb/stb_ds.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>


void MarkValue(Pgc * gc, PValue value);
void MarkObject(Pgc * gc, PObj * obj);
void MarkObjectChilds(Pgc * gc, PObj * obj);

Pgc *NewGc(void) {
    Pgc *gc = PCreate(Pgc);
    gc->disable = false;
    gc->stress = false;
    gc->nextGc = 1024 * 1024;
    gc->objects = NULL;
    gc->stmts = NULL;
    gc->timestamp = (uint64_t)time(NULL);

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

    PFree(gc);
}


void MarkValue(Pgc * gc, PValue value){
	if (IsValueObj(value)) {
		MarkObject(gc, ValueAsObj(value));
	}
}


void MarkObject(Pgc * gc, PObj * obj){
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
	MarkObjectChilds(gc, obj);
}

void MarkEnv(Pgc * gc, PEnv * env){
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
		MarkValue(gc, env->table[i].value);
	}
}



void MarkObjectChilds(Pgc * gc, PObj * obj){
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
				MarkValue(gc, arr->items[i]);
			}
			break;
		}

		case OT_MAP:{
			struct OMap * map = &obj->v.OMap;
			size_t count = map->count;
			for (size_t i = 0; i < count; i++) {
				MarkValue(gc, map->table[i].vkey);
				MarkValue(gc, map->table[i].value);
			}
			break;
			
		}
		case OT_FNC:{
			struct OFunction * func = &obj->v.OFunction;
			if (func->body) {
				//MarkStatements
			}

			if (func->env != NULL) {
				MarkEnv(gc, func->env);
			}
			
			break;

		}

	}
}
