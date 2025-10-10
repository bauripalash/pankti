#include "include/object.h"
#include "include/alloc.h"
#include "include/utils.h"
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

bool IsObjTruthy(const PObj * o){
	if (o != NULL && o->type == OT_BOOL) {
		return o->v.bl;
	}

	return false;
}

bool isObjEqual(const PObj * a, const PObj * b){
	if (a->type != b->type) {
		return false;
	}
	bool result = false;
	switch (a->type) {
		case OT_NUM:{
			result = (a->v.num == b->v.num);
			break;
		}
		case OT_BOOL:{
			result = (a->v.bl == b->v.bl);
			break;
		}

		case OT_NIL:{
			result = true;
			break;
		}

		case OT_STR:{
			result = StrEqual(a->v.str, b->v.str);
			break;
		}
	}

	return result;
}
