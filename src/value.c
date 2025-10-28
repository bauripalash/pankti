#include "include/value.h"
#include "include/object.h"
#include <stdio.h>

void PrintValue(const PValue * val){
	switch (val->type) {
		case VT_NUM: printf("%g", val->v.num);break;
		case VT_BOOL: printf("%s" , val->v.bl ? "true" : "false");break;
		case VT_NIL: printf("nil");break;
		case VT_OBJ: PrintObject(val->v.obj);break;
	}
}
