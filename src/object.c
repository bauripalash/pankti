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
		case OT_RET:{
			printf("<ret "); 
			PrintObject(o->v.OReturn.rvalue);
			printf(">");
			break;
		}
		case OT_BRK:{printf("<break>\n");break;}
	}
}


char * ObjTypeToString(PObjType type){
	switch (type) {
		case OT_NUM: return "Number";break;
		case OT_STR: return "String";break;
		case OT_BOOL: return "Bool";break;
		case OT_NIL: return "Nil";break;
		case OT_RET: return "Return";break;
		case OT_BRK: return "Break";break;
	}

	return "";
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

PObj * NewNilObject(){
	return NewObject(OT_NIL);
}


PObj * NewReturnObject(PObj * value){
	PObj * o = NewObject(OT_RET);
	o->v.OReturn.rvalue = value;
	return o;
}


PObj * NewBreakObject(){
	PObj * o = NewObject(OT_BRK);
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
		case OT_RET:{
			result = isObjEqual(a->v.OReturn.rvalue, a->v.OReturn.rvalue);
			break;
		}

		case OT_BRK:{
			result = true;
			break;
		}

	}

	return result;
}
