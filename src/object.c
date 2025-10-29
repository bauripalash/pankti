#include "include/object.h"
#include "include/utils.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void PrintValue(const PValue * val){
	switch (val->type) {
		case VT_NUM: printf("%g", val->v.num);break;
		case VT_BOOL: printf("%s" , val->v.bl ? "true" : "false");break;
		case VT_NIL: printf("nil");break;
		case VT_OBJ: PrintObject(val->v.obj);break;
	}
}

bool IsValueTruthy(const PValue * val){
	if (val->type == VT_BOOL && val->v.bl) {
		return true;
	}else{
		return false;
	}
}

bool IsValueEqual(const PValue * a, const PValue * b){
	if (a == NULL || b == NULL) {
		return false;
	}

	if (a->type != b->type) {
		return false;
	}

	if (a->type == VT_OBJ) {
		return IsObjEqual(a->v.obj, b->v.obj);
	} else if (a->type == VT_NIL) {
		return true;
	}else if (a->type == VT_NUM) {
		return a->v.num == b->v.num;
	}else if (a->type == VT_BOOL) {
		return a->v.bl == b->v.bl;
	}
	return false;
	
}

void PrintObject(const PObj *o) {
    if (o == NULL) {
        return;
    }

    switch (o->type) {
        case OT_NUM: printf("%f", o->v.num); break;
        case OT_STR: printf("%s", o->v.str); break;
        case OT_BOOL: printf("%s", o->v.bl ? "true" : "false"); break;
        case OT_NIL: printf("nil"); break;
        case OT_RET: {
            printf("<ret ");
            PrintObject(o->v.OReturn.rvalue);
            printf(">");
            break;
        }
        case OT_BRK: {
            printf("<break>\n");
            break;
        }
        case OT_FNC: {
            printf("<fn %s>", o->v.OFunction.name->lexeme);
            break;
        }
    }
}

char *ObjTypeToString(PObjType type) {
    switch (type) {
        case OT_NUM: return "Number"; break;
        case OT_STR: return "String"; break;
        case OT_BOOL: return "Bool"; break;
        case OT_NIL: return "Nil"; break;
        case OT_RET: return "Return"; break;
        case OT_BRK: return "Break"; break;
        case OT_FNC: return "Function"; break;
    }

    return "";
}

bool IsObjTruthy(const PObj *o) {
    if (o != NULL && o->type == OT_BOOL) {
        return o->v.bl;
    }

    return false;
}

bool IsObjEqual(const PObj *a, const PObj *b) {
    if (a->type != b->type) {
        return false;
    }
    bool result = false;
    switch (a->type) {
        case OT_NUM: {
            result = (a->v.num == b->v.num);
            break;
        }
        case OT_BOOL: {
            result = (a->v.bl == b->v.bl);
            break;
        }

        case OT_NIL: {
            result = true;
            break;
        }

        case OT_STR: {
            result = StrEqual(a->v.str, b->v.str);
            break;
        }
        case OT_RET: {
            result = IsObjEqual(a->v.OReturn.rvalue, a->v.OReturn.rvalue);
            break;
        }

        case OT_BRK: {
            result = true;
            break;
        }

        case OT_FNC: result = false; break;
    }

    return result;
}
