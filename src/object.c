#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "external/stb/stb_ds.h"
#include "external/xxhash/xxhash.h"
#include "include/keywords.h"
#include "include/object.h"
#include "include/printer.h"
#include "include/ptypes.h"
#include "include/utils.h"

// Golden ratio?
#define CONST_NAN_HASH 0x9e3779b97f4a7c15ULL
// MurmurHash3
#define CONST_ZERO_HASH       0xff51afd7ed558ccdULL

#define CONST_BOOL_TRUE_HASH  0x1
#define CONST_BOOL_FALSE_HASH 0x0
#define CONST_NIL_HASH        0x2

void PrintValue(PValue val) {
    if (IsValueNum(val)) {
        double num = ValueAsNum(val);

        if (IsDoubleInt(num)) {
            PanPrint("%0.f", num);
        } else {
            PanPrint("%f", num);
        }
    } else if (IsValueBool(val)) {
        PanPrint("%s", ValueAsBool(val) ? KW_BN_TRUE : KW_BN_FALSE);
    } else if (IsValueNil(val)) {
        PanPrint(KW_BN_NIL);
    } else if (IsValueObj(val)) {
        PrintObject(ValueAsObj(val));
    } else {
        PanPrint("Unknown");
    }
}

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
        case PVAL_NUM: return "Number";
        case PVAL_BOOL: return "Bool";
        case PVAL_NIL: return "Nil";
        case PVAL_OBJ: return ObjTypeToString(ValueAsObj(val)->type);
    }

    return "Unknown";
}

bool CanObjectBeKey(PObjType type) {
    if (type == OT_STR) {
        return true;
    }

    return false;
}

bool CanValueBeKey(PValue val) {
    if (IsValueObj(val)) {
        return CanObjectBeKey(ValueAsObj(val)->type);
    }

    return true;
}

u64 GetObjectHash(const PObj *obj, u64 seed) {
    if (obj->type == OT_STR) {
        XXH64_hash_t hash =
            XXH64(obj->v.OString.value, strlen(obj->v.OString.value), seed);
        return (u64)hash;
    }

    // Function should never reach here. Runtime checks should check for types
    return 0;
}

u64 GetValueHash(PValue val, u64 seed) {
    if (IsValueNum(val)) {
        double value = ValueAsNum(val);
        if (value == 0.0) {
            return CONST_ZERO_HASH;
        }
        if (isnan(value)) {
            return CONST_ZERO_HASH;
        }
        u64 bits;
        memcpy(&bits, &value, sizeof(bits));
        return (u64)XXH64(&bits, sizeof(bits), seed);
    } else if (IsValueNil(val)) {
        return CONST_NIL_HASH;
    } else if (IsValueBool(val)) {
        u8 value = ValueAsBool(val) ? 1 : 0;
        return (u64)XXH64(&value, 1, seed);
    } else if (IsValueObj(val)) {
        return GetObjectHash(ValueAsObj(val), seed);
    }

    return UINT64_MAX;
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

bool IsValueError(PValue val) { return IsValueObjType(val, OT_ERROR); }

char *GetErrorObjMsg(PObj *obj) {
    if (obj->type == OT_ERROR && obj->v.OError.msg != NULL) {
        return obj->v.OError.msg;
    }

    return NULL;
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

bool ArrayObjInsValue(PObj *o, int index, PValue value) {
    if (o == NULL) {
        return false;
    }

    if (o->type != OT_ARR) {
        return false;
    }

    struct OArray *arr = &o->v.OArray;
    if (index < 0 || index >= arr->count) {
        return false;
    }

    arrput(arr->items, value);
    arrdelswap(arr->items, index);
    arr->count = (u64)arrlen(arr->items);

    return true;
}

bool ArrayObjPushValue(PObj *o, PValue value) {
    if (o == NULL || o->type != OT_ARR) {
        return false;
    }

    struct OArray *arr = &o->v.OArray;
    u64 oldCount = arr->count;
    arrput(arr->items, value);
    u64 newCount = arrlen(arr->items);

    if (newCount == oldCount + 1) {
        arr->count = newCount;
        return true;
    } else {
        return false;
    }
}

bool MapObjSetValue(PObj *o, PValue key, u64 keyHash, PValue value) {
    if (o == NULL) {
        return false;
    }

    if (o->type != OT_MAP) {
        return false;
    }

    struct OMap *map = &o->v.OMap;
    MapEntry s = (MapEntry){keyHash, key, value};
    hmputs(map->table, s);
    map->count = (u64)hmlen(map->table);
    return true;
}

void PrintObject(const PObj *o) {
    if (o == NULL) {
        return;
    }

    switch (o->type) {
        case OT_STR: {
            const struct OString *str = &o->v.OString;
            if (str->value != NULL) {
                PanPrint("%s", str->value);
            } else {
                PanPrint("<INVALID STRING>");
            }

            break;
        }
        case OT_FNC: {
            const struct OFunction *func = &o->v.OFunction;
            if (func->name != NULL) {
                PanPrint("<fn %s>", func->name->lexeme);
            }
            break;
        }
        case OT_ARR: {
            const struct OArray *arr = &o->v.OArray;
            PanPrint("[");
            if (arr->items != NULL) {
                for (u64 i = 0; i < arrlen(arr->items); i++) {
                    PValue val = arr->items[i];
                    PrintValue(val);
                    if (i != arr->count - 1) {
                        PanPrint(", ");
                    }
                }
            }
            PanPrint("]");
            break;
        }
        case OT_NATIVE: {
            PanPrint(
                "<native %s>",
                o->v.ONative.name != NULL ? o->v.ONative.name->lexeme : "'Null'"
            );
            break;
        }
        case OT_MAP: {
            PanPrint("{");
            const struct OMap *map = &o->v.OMap;
            if (map->table != NULL) {
                for (int i = 0; i < map->count; i++) {
                    PValue k = map->table[i].vkey;
                    PValue v = map->table[i].value;
                    PrintValue(k);
                    PanPrint(" : ");
                    PrintValue(v);
                    if (i + 1 != map->count) {
                        PanPrint(", ");
                    }
                }
            }

            PanPrint("}");
            break;
        }
        case OT_ERROR: {
            if (o->v.OError.msg != NULL) {
                PanPrint("%s", o->v.OError.msg);
            }
            break;
        }
        case OT_UPVAL: {
            PrintValue(o->v.OUpval.value);
            break;
        }
    }
}

char *ObjTypeToString(PObjType type) {
    switch (type) {
        case OT_STR: return "String";
        case OT_FNC: return "Function";
        case OT_ARR: return "Array";
        case OT_MAP: return "HashMap";
        case OT_NATIVE: return "Native Func";
        case OT_ERROR: return "Error";
        case OT_UPVAL: return "Upvalue";
    }
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
        case OT_FNC: result = false; break;
        case OT_ARR: result = false; break; // TODO: fix
        case OT_NATIVE: result = (a->v.ONative.fn == b->v.ONative.fn); break;
        case OT_MAP: result = false; break;
        case OT_ERROR: {
            if (a->v.OError.msg != NULL && b->v.OError.msg != NULL) {
                result = StrEqual(a->v.OError.msg, b->v.OError.msg);
            } else {
                result = false;
            }
            break;
        }
        case OT_UPVAL: {
            result = IsValueEqual(a->v.OUpval.value, b->v.OUpval.value);
            break;
        }
    }

    return result;
}
