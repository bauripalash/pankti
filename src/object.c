#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "external/gb/gb_string.h"
#include "external/stb/stb_ds.h"
#include "external/xxhash/xxhash.h"
#include "include/alloc.h"
#include "include/object.h"
#include "include/panktiterms.h"
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

#define NUM_STR_BUF_SIZE      64

void PrintValue(PValue val) {
    if (IsValueNum(val)) {
        double num = ValueAsNum(val);
        if (IsDoubleInt(num)) {
            PanPrint("%0.f", num);
        } else {
            char buf[NUM_STR_BUF_SIZE];
            FormatDouble(num, buf, NUM_STR_BUF_SIZE);
            PanPrint("%s", buf);
        }
    } else if (IsValueBool(val)) {
        PanPrint("%s", ValueAsBool(val) ? PANTERM_TRUE : PANTERM_FALSE);
    } else if (IsValueNil(val)) {
        PanPrint(PANTERM_NIL);
    } else if (IsValueObj(val)) {
        PrintObject(ValueAsObj(val));
    } else {
        PanPrint(PANTERM_UNKNOWN);
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
        case PVAL_NUM: return PANTERM_NUMBER;
        case PVAL_BOOL: return PANTERM_BOOLEAN;
        case PVAL_NIL: return PANTERM_NIL;
        case PVAL_OBJ: return ObjTypeToString(ValueAsObj(val)->type);
    }

    return PANTERM_UNKNOWN;
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
            return CONST_NAN_HASH;
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

bool MapObjPushPair(PObj *o, PValue key, PValue value, u64 seed) {
    u64 keyHash = GetValueHash(key, seed);
    return MapObjSetValue(o, key, keyHash, value);
}

bool MapObjHasKey(PObj *o, PValue key, u64 hash) {
    assert(o->type == OT_MAP);

    if (hmgeti(o->v.OMap.table, hash) >= 0) {
        return true;
    }

    return false;
}

PValue MapObjGetValue(PObj *map, PValue key, u64 keyHash, bool *found) {
    assert(map->type == OT_MAP);

    if (hmgeti(map->v.OMap.table, keyHash) >= 0) {
        *found = true;
        return hmgets(map->v.OMap.table, keyHash).value;
    }

    *found = false;
    return MakeNil();
}

PValue MapObjRemoveKey(PObj *map, PValue key, u64 keyHash, bool *ok) {
    assert(map->type == OT_MAP);
    if (map == NULL || ok == NULL) {
        return MakeNil();
    }
    struct OMap *mapobj = &map->v.OMap;

    bool found = false;
    PValue result = MapObjGetValue(map, key, keyHash, &found);

    if (!found) {
        *ok = false;
        return MakeNil();
    }

    if (hmdel(mapobj->table, keyHash)) {
        mapobj->count = (u64)hmlen(mapobj->table);
        *ok = true;
        return result;
    }

    *ok = false;
    return MakeNil();
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
                PanPrint("<" PANTERM_UNKNOWN ">");
            }

            break;
        }
        case OT_COMFNC: {
            const struct OComFunction *func = &o->v.OComFunction;
            if (func->strName != NULL) {
                PanPrint(
                    "<%s '%s'>", PANTERM_FUNCTION,
                    func->strName->v.OString.value
                );
            } else {
                PanPrint("<%s>", PANTERM_SCRIPT);
            }
            break;
        }
        case OT_CLOSURE: {
            const struct OClosure *cls = &o->v.OClosure;
            PrintObject(cls->function);
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
                "<%s '%s'>", PANTERM_NATIVE_FUNC,
                o->v.ONative.name != NULL ? o->v.ONative.name : PANTERM_UNKNOWN
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
        case OT_MODULE: {
            char *name = NULL;
            const struct OModule *mod = &o->v.OModule;
            if (mod->customName != NULL) {
                name = mod->customName;
            }

            PanPrint(
                "<%s '%s'>", PANTERM_MODULE,
                name != NULL ? name : PANTERM_UNKNOWN
            );
            break;
        }
        case OT_UPVAL: {
            PanPrint("<" PANTERM_UPVALUE ">");
            break;
        }
    }
}

char *ValueToString(PValue val) {
    char *result = NULL;
    if (IsValueNum(val)) {
        gbString str;
        double dblNum = ValueAsNum(val);
        if (IsDoubleInt(dblNum)) {
            str = gb_make_string(StrFormat("%d", (int)floor(dblNum)));
        } else {
            char buf[NUM_STR_BUF_SIZE];
            FormatDouble(dblNum, buf, NUM_STR_BUF_SIZE);
            str = gb_make_string(StrFormat("%s", buf));
        }
        result = StrDuplicate(str, (u64)gb_string_length(str));
        gb_free_string(str);
        return result;

    } else if (IsValueBool(val)) {
        if (ValueAsBool(val) == true) { // just make it more verbose
            result = StrDuplicate(PANTERM_TRUE, StrLength(PANTERM_TRUE));
        } else {
            result = StrDuplicate(PANTERM_FALSE, StrLength(PANTERM_FALSE));
        }
    } else if (IsValueNil(val)) {
        result = StrDuplicate(PANTERM_NIL, StrLength(PANTERM_NIL));
    } else if (IsValueObj(val)) {
        result = ObjToString(ValueAsObj(val));
    }

    return result;
}

char *ObjToString(PObj *obj) {
    char *result = NULL;
    switch (obj->type) {
        case OT_STR: {
            struct OString *str = &obj->v.OString;
            if (str->value == NULL) {
                return NULL;
            }

            result = StrDuplicate(str->value, StrLength(str->value));
            break;
        }
        case OT_COMFNC: {
            struct OComFunction *fn = &obj->v.OComFunction;
            if (fn->strName != NULL) {
                const char *temp = StrFormat(
                    "<%s '%s'>", PANTERM_FUNCTION,
                    fn->strName != NULL ? fn->strName->v.OString.value
                                        : PANTERM_UNKNOWN
                );
                result = StrDuplicate(temp, StrLength(temp));
            } else {
                result = StrDuplicate(
                    "<" PANTERM_SCRIPT ">", StrLength("<" PANTERM_SCRIPT ">")
                );
            }

            break;
        }
        case OT_CLOSURE: {
            struct OClosure *cls = &obj->v.OClosure;
            result = ObjToString(cls->function);
            break;
        }
        case OT_ARR: {
            struct OArray *arr = &obj->v.OArray;

            gbString s = gb_make_string("[");

            for (u64 i = 0; i < arr->count; i++) {
                PValue val = arr->items[i];
                char *temp = ValueToString(val);
                s = gb_append_cstring(s, temp);

                if (i != arr->count - 1) {
                    s = gb_append_cstring(s, ", ");
                }
                PFree(temp);
            }
            s = gb_append_cstring(s, "]");
            result = StrDuplicate(s, (u64)gb_string_length(s));
            gb_free_string(s);
            break;
        }
        case OT_MAP: {
            struct OMap *map = &obj->v.OMap;
            gbString s = gb_make_string("{");
            u64 count = map->count;
            for (u64 i = 0; i < count; i++) {
                PValue key = map->table[i].vkey;
                PValue val = map->table[i].value;

                char *keyStr = ValueToString(key);
                char *valStr = ValueToString(val);

                if (keyStr == NULL || valStr == NULL) {
                    gb_free_string(s);
                    PFree(result);
                    result = NULL;

                    if (keyStr != NULL) {
                        PFree(keyStr);
                    }

                    if (valStr != NULL) {
                        PFree(valStr);
                    }

                    return NULL;
                }

                s = gb_append_cstring(s, StrFormat("%s: %s", keyStr, valStr));
                if (i != count - 1) {
                    s = gb_append_cstring(s, ", ");
                }
                PFree(keyStr);
                PFree(valStr);
            }

            s = gb_append_cstring(s, "}");
            result = StrDuplicate(s, (u64)gb_string_length(s));
            gb_free_string(s);
            break;
        }
        case OT_NATIVE: {
            struct ONative *nfn = &obj->v.ONative;
            const char *temp = StrFormat(
                "<%s '%s'>", PANTERM_NATIVE_FUNC,
                nfn->name != NULL ? nfn->name : PANTERM_UNKNOWN
            );
            result = StrDuplicate(temp, strlen(temp));
            break;
        }
        case OT_MODULE: {
            const char *name = NULL;
            bool hasName = false;
            if (obj->v.OModule.customName != NULL) {
                name = obj->v.OModule.customName;
                hasName = true;
            }
            const char *temp = StrFormat(
                "<%s '%s'>", PANTERM_MODULE, hasName ? name : PANTERM_UNKNOWN
            );
            result = StrDuplicate(temp, StrLength(temp));
            break;
        }
        case OT_UPVAL: {
            const char *temp = StrFormat("<" PANTERM_UPVALUE ">");
            result = StrDuplicate(temp, strlen(temp));
            break;
        }
    }

    return result;
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
