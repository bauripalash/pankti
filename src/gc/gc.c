#include "../include/gc.h"
#include "../include/alloc.h"
#include <stdbool.h>
#include <stdio.h>

Pgc *NewGc() {
    Pgc *gc = PCreate(Pgc);
    gc->disable = false;
    gc->stress = false;
    gc->nextGc = 1024 * 1024;
    gc->objects = NULL;
    gc->stmts = NULL;

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
