#include "../include/gc.h"
#include "../include/alloc.h"
#include <stdbool.h>
#include <stdio.h>

static PObj * makeOnlyBool(bool value){
	PObj * o = PCreate(PObj);
	o->type = OT_BOOL;
	o->v.bl = value;
	o->next = NULL;
	return o;
}

static PObj * makeOnlyNil(){
	PObj * o = PCreate(PObj);
	o->type = OT_NIL;
	o->next = NULL;
	return o;
}

Pgc * NewGc(){
	Pgc * gc = PCreate(Pgc);
	gc->disable = false;
	gc->stress = false;
	gc->nextGc = 1024 * 1024;
	gc->objects = NULL;

	gc->onlyNilObj = makeOnlyNil();
	gc->onlyTrueObj = makeOnlyBool(true);
	gc->onlyFalseObj = makeOnlyBool(false);

	return gc;
}
void FreeGc(Pgc * gc){
	if (gc == NULL) {
		return;
	}

	FreeObject(gc, gc->onlyNilObj);
	FreeObject(gc, gc->onlyTrueObj);
	FreeObject(gc, gc->onlyFalseObj);

	PFree(gc);
}
