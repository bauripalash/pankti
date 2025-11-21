#include "include/object.h"
#include "external/stb/stb_ds.h"
#include "external/xxhash/xxhash.h"
#include "include/utils.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


// Golden ratio?
#define CONST_NAN_HASH 0x9e3779b97f4a7c15ULL
// MurmurHash3
#define CONST_ZERO_HASH       0xff51afd7ed558ccdULL

#define CONST_BOOL_TRUE_HASH  0x1
#define CONST_BOOL_FALSE_HASH 0x0
#define CONST_NIL_HASH        0x2

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

bool CanObjectBeKey(PObjType type) {
    if (type == OT_STR) {
        return true;
    }

    return false;
}

bool CanValueBeKey(const PValue *val) {
    if (val->type == VT_OBJ) {
        return CanObjectBeKey(val->v.obj->type);
    }

    return true;
}

uint64_t GetObjectHash(const PObj *obj, uint64_t seed) {
    if (obj->type == OT_STR) {
        XXH64_hash_t hash =
            XXH64(obj->v.OString.value, strlen(obj->v.OString.value), seed);
        return (uint64_t)hash;
    }

	// Function should never reach here. Runtime checks should check for types
    return UINT64_MAX;
}

uint64_t GetValueHash(const PValue *val, uint64_t seed) {
    switch (val->type) {
        case VT_NUM: {
            double value = val->v.num;
            if (value == 0.0) {
                return CONST_ZERO_HASH;
            }
            if (isnan(value)) {
                return CONST_ZERO_HASH;
            }
            uint64_t bits;
            memcpy(&bits, &value, sizeof(bits));
            return (uint64_t)XXH64(&bits, sizeof(bits), seed);
        }
        case VT_NIL: {
            return CONST_NIL_HASH;
        }
        case VT_BOOL: {
            uint8_t value = val->v.bl ? 1 : 0;
            return (uint64_t)XXH64(&value, 1, seed);
        };
        case VT_OBJ: return GetObjectHash(val->v.obj, seed);
    }
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

    return true;
}

bool MapObjSetValue(PObj *o, PValue key, uint64_t keyHash, PValue value) {
    if (o == NULL) {
        return false;
    }

    if (o->type != OT_MAP) {
        return false;
    }

    struct OMap *map = &o->v.OMap;
    MapEntry s = (MapEntry){keyHash, key, value};
    hmputs(map->table, s);
    map->count = hmlen(map->table);
    return true;
}

void PrintObject(const PObj *o) {
    if (o == NULL) {
        return;
    }

    switch (o->type) {
        case OT_STR: printf("%s", o->v.OString.value); break;
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
        case OT_NATIVE: {
            printf(
                "<native %s>",
                o->v.ONative.name != NULL ? o->v.ONative.name->lexeme : "'Null'"
            );
            break;
        }
        case OT_MAP: {
            printf("{");
            const struct OMap *map = &o->v.OMap;
            for (int i = 0; i < map->count; i++) {
                PValue *k = &map->table[i].vkey;
                PValue *v = &map->table[i].value;
                PrintValue(k);
                printf(" : ");
                PrintValue(v);
                if (i + 1 != map->count) {
                    printf(", ");
                }
            }
            printf("}");
            break;
        }
    }
}

char *ObjTypeToString(PObjType type) {
    switch (type) {
        case OT_STR: return "String"; break;
        case OT_FNC: return "Function"; break;
        case OT_ARR: return "Array"; break;
        case OT_MAP: return "HashMap"; break;
        case OT_NATIVE: return "Native Func"; break;
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
            result = StrEqual(a->v.OString.value, b->v.OString.value);
            break;
        }
        case OT_FNC: result = false; break;
        case OT_ARR: result = false; break; // TODO: fix
        case OT_NATIVE: result = (a->v.ONative.fn == b->v.ONative.fn); break;
        case OT_MAP: result = false; break;
    }

    return result;
}
