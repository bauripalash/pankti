#include "include/object.h"
#include "include/utils.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void PrintValue(const PValue *val) {
    switch (val->type) {
        case VT_NUM: printf("%f", val->v.num); break;
        case VT_BOOL: printf("%s", val->v.bl ? "true" : "false"); break;
        case VT_NIL: printf("nil"); break;
        case VT_OBJ: PrintObject(val->v.obj); break;
    }
}

const char *ValueTypeToStr(const PValue *val) {
    if (val == NULL) {
        return "Unknown";
    }

    switch (val->type) {
        case VT_NUM: return "Number";
        case VT_BOOL: return "Bool";
        case VT_NIL: return "Nil";
        case VT_OBJ: return ObjTypeToString(val->v.obj->type);
    }

    return "Unknown";
}

bool IsValueTruthy(const PValue *val) {
    if (val->type == VT_BOOL && val->v.bl) {
        return true;
    } else {
        return false;
    }
}

bool IsValueEqual(const PValue *a, const PValue *b) {
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
    } else if (a->type == VT_NUM) {
        return a->v.num == b->v.num;
    } else if (a->type == VT_BOOL) {
        return a->v.bl == b->v.bl;
    }
    return false;
}

void PrintObject(const PObj *o) {
    if (o == NULL) {
        return;
    }

    switch (o->type) {
        case OT_STR: printf("%s", o->v.str); break;
        case OT_FNC: {
            printf("<fn %s>", o->v.OFunction.name->lexeme);
            break;
        }
        case OT_ARR: {
            const struct OArray *arr = &o->v.OArray;
            printf("[");
            for (int i = 0; i < arr->count; i++) {
                PrintValue(&arr->items[i]);
                if (i != arr->count - 1) {
                    printf(", ");
                }
            }
            printf("]");
            break;
        }
    }
}

char *ObjTypeToString(PObjType type) {
    switch (type) {
        case OT_STR: return "String"; break;
        case OT_FNC: return "Function"; break;
        case OT_ARR: return "Array"; break;
    }

    return "";
}

bool IsObjEqual(const PObj *a, const PObj *b) {
    if (a->type != b->type) {
        return false;
    }
    bool result = false;
    switch (a->type) {
        case OT_STR: {
            result = StrEqual(a->v.str, b->v.str);
            break;
        }
        case OT_FNC: result = false; break;
        case OT_ARR: result = false; break; // TODO: fix
    }

    return result;
}
