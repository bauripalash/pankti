#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "../include/object.h"
#include "../include/panktiterms.h"
#include "../include/utils.h"

PValueType GetValueType(PValue value) {
    if (IsValueNum(value)) {
        return PVAL_NUM;
    } else if (IsValueBool(value)) {
        return PVAL_BOOL;
    } else if (IsValueNil(value)) {
        return PVAL_NIL;
    } else {
        return PVAL_OBJ;
    }
}

const char *ValueTypeToStr(PValue val) {
    switch (GetValueType(val)) {
        case PVAL_NUM: return PANTERM_NUMBER;
        case PVAL_BOOL: return PANTERM_BOOLEAN;
        case PVAL_NIL: return PANTERM_NIL;
        case PVAL_OBJ: return ObjTypeToString(ValueAsObj(val)->type);
    }

    return PANTERM_UNKNOWN;
}

bool IsValueTruthy(PValue val) {
    if (IsValueBool(val)) {
        return ValueAsBool(val);
    } else {
        return false;
    }
}

bool IsValueEqual(PValue a, PValue b) {
    if (GetValueType(a) != GetValueType(b)) {
        return false;
    }

    if (IsValueObj(a)) {
        return IsObjEqual(ValueAsObj(a), ValueAsObj(b));
    } else {
#if defined(USE_NAN_BOXING)
        if (IsValueNum(a)) {
            return ValueAsNum(a) == ValueAsNum(b);
        }
        return a == b;
#else
        if (a.type == PVAL_NIL) {
            return true;
        } else if (a.type == PVAL_NUM) {
            return a.v.num == b.v.num;
        } else if (a.type == PVAL_BOOL) {
            return a.v.bl == b.v.bl;
        }
#endif
    }

    return false;
}

bool ObjectHasLen(PObj *obj) {
    if (obj == NULL) {
        return false;
    }
    PObjType ot = obj->type;

    if (ot == OT_STR || ot == OT_ARR || ot == OT_MAP) {
        return true;
    }

    return false;
}

double GetObjectLength(PObj *obj) {

    if (obj->type == OT_ARR) {
        return (double)obj->v.OArray.count;
    } else if (obj->type == OT_MAP) {
        return (double)obj->v.OMap.count;
    } else if (obj->type == OT_STR) {
        return (double)StrLength(obj->v.OString.value);
    }

    return -1; // Should never reach here
}

char *ObjTypeToString(PObjType type) {
    switch (type) {
        case OT_STR: return PANTERM_STRING;
        case OT_ARR: return PANTERM_ARRAY;
        case OT_MAP: return PANTERM_MAP;
        case OT_NATIVE: return PANTERM_NATIVE_FUNC;
        case OT_MODULE: return PANTERM_MODULE;
        case OT_UPVAL: return PANTERM_UPVALUE;
        case OT_COMFNC:
            return PANTERM_FUNCTION;
            // To users functions and closures are not different
        case OT_CLOSURE: return PANTERM_FUNCTION;
    }
    return PANTERM_UNKNOWN; // should never reach here
}

bool IsObjEqual(const PObj *a, const PObj *b) {
    if (a->type != b->type) {
        return false;
    }
    bool result = false;
    switch (a->type) {
        case OT_STR: {
            result = StrEqual(a->v.OString.value, b->v.OString.value);
            break;
        }

        case OT_COMFNC: result = false; break;
        case OT_CLOSURE: result = false; break;
        case OT_ARR: result = false; break; // TODO: fix
        case OT_NATIVE: result = (a->v.ONative.fn == b->v.ONative.fn); break;
        case OT_MAP: result = false; break;
        case OT_MODULE: {
            if (a->v.OModule.path != NULL && b->v.OModule.path != NULL) {
                result = StrEqual(a->v.OModule.path, b->v.OModule.path);
            } else {
                result = false;
            }

            break;
        }
        case OT_UPVAL: {
            result = false;
            // result = IsValueEqual(a->v.OUpval.value, b->v.OUpval.value);
            break;
        }
    }

    return result;
}
