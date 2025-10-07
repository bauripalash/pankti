#include "include/object.h"
#include "include/alloc.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

PObj * NewObject(PObjType type){
	PObj * o = PCreate(PObj);
	o->type = type;
	return o;
}
void FreeObject(PObj * o){
	if (o == NULL) {
		return;
	}

	free(o);
}
void PrintObject(const PObj * o){
	if (o == NULL) {
		return;
	}

	switch (o->type) {
		case OT_NUM: printf("%f", o->v.num);break;
		case OT_STR: printf("%s", o->v.str);break;
		case OT_BOOL: printf("%s", o->v.bl ? "true" : "false");break;
		case OT_NIL: printf("nil");break;
	}
}

PObj * NewNumberObj(double value){
	PObj * o = NewObject(OT_NUM);
	o->v.num = value;
	return o;
}

PObj * NewBoolObj(bool value){
	PObj * o = NewObject(OT_BOOL);
	o->v.bl = value;
	return o;
}

PObj * NewStrObject(char * value){
	PObj * o = NewObject(OT_STR);
	o->v.str = value;
	return o;
}
