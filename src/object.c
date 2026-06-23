/*
 * Copyright (c) 2022 Palash Bauri
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "object.h"
#include "panktiterms.h"
#include "utils.h"

typedef struct ObjSeenPair {
    const PObj *a[OBJ_SEEN_CAP];
    const PObj *b[OBJ_SEEN_CAP];
    int len;
} ObjSeenPair;

static bool internalIsValueEqual(PValue a, PValue b, ObjSeenPair *pair);
static bool internalIsObjEqual(const PObj *a, const PObj *b, ObjSeenPair *pair);

static bool SeenPairEnter(ObjSeenPair *pair, const PObj *a, const PObj *b) {
    for (int i = 0; i < pair->len; i++) {
        if (pair->a[i] == a && pair->b[i] == b) {
            return true;
        }
    }
    if (pair->len < OBJ_SEEN_CAP) {
        pair->a[pair->len] = a;
        pair->b[pair->len] = b;
        pair->len++;
    }
    return false;
}

static void SeenPairExit(ObjSeenPair *pair, const PObj *a, const PObj *b) {
    for (int i = 0; i < pair->len; i++) {
        if (pair->a[i] == a && pair->b[i] == b) {
            pair->a[i] = pair->a[pair->len - 1];
            pair->b[i] = pair->b[pair->len - 1];
            pair->len--;
            return;
        }
    }
}

bool SeenSetEnter(ObjSeenSet *set, const PObj *obj) {
    for (int i = 0; i < set->len; i++) {
        if (set->buf[i] == obj) {
            return true; // already seen (cyclic)
        }
    }
    if (set->len < OBJ_SEEN_CAP) {
        set->buf[set->len++] = obj;
    }
    return false;
}
void SeenSetExit(ObjSeenSet *set, const PObj *obj) {
    for (int i = 0; i < set->len; i++) {
        if (set->buf[i] == obj) {
            set->buf[i] = set->buf[--set->len]; // swap last and shrink len
        }
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

bool IsValueTruthy(PValue val) {
    if (IsValueBool(val)) {
        return ValueAsBool(val);
    } else {
        return false;
    }
}

bool IsValueObjType(PValue val, PObjType otype) {
    if (!IsValueObj(val)) {
        return false;
    }

    PObj *obj = ValueAsObj(val);
    if (obj->type == otype) {
        return true;
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

bool IsValueEqual(PValue a, PValue b) {
    ObjSeenPair pair = {0};
    return internalIsValueEqual(a, b, &pair);
}

bool IsObjEqual(const PObj *a, const PObj *b) {
    ObjSeenPair pair = {0};
    return internalIsObjEqual(a, b, &pair);
}

static bool internalIsValueEqual(PValue a, PValue b, ObjSeenPair *pair) {
    if (GetValueType(a) != GetValueType(b)) {
        return false;
    }

    if (IsValueObj(a)) {
        return internalIsObjEqual(ValueAsObj(a), ValueAsObj(b), pair);
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

static bool internalIsObjEqual(
    const PObj *a, const PObj *b, ObjSeenPair *pair
) {
    if (a->type != b->type) {
        return false;
    }
    switch (a->type) {
        case OT_COMFNC:
        case OT_CLOSURE:
        case OT_UPVAL: {
            return false;
        }
        case OT_STR: {
            return StrEqual(a->v.OString.value, b->v.OString.value);
        }
        case OT_NATIVE: {
            return (a->v.ONative.fn == b->v.ONative.fn);
        }
        case OT_MODULE: {
            if (a->v.OModule.path != NULL && b->v.OModule.path != NULL) {
                return StrEqual(a->v.OModule.path, b->v.OModule.path);
            } else {
                return false;
            }
        }

        case OT_ARR: {
            const struct OArray *arrA = &a->v.OArray;
            const struct OArray *arrB = &b->v.OArray;

            if (arrA->count != arrB->count) {
                return false;
            }

            // checks if we already compared these objects in this cycle
            if (SeenPairEnter(pair, a, b)) {
                return true;
            }

            bool result = true;
            for (u64 i = 0; i < arrA->count; i++) {
                if (!internalIsValueEqual(
                        arrA->items[i], arrB->items[i], pair
                    )) {
                    result = false;
                    break;
                }
            }
            SeenPairExit(pair, a, b);
            return result;
        }
        case OT_MAP: {
            const struct OMap *mapA = &a->v.OMap;
            const struct OMap *mapB = &b->v.OMap;

            if (mapA->count != mapB->count) {
                return false;
            }

            // checks if we already compared these objects in this cycle
            if (SeenPairEnter(pair, a, b)) {
                return true;
            }

            bool result = true;

            for (u64 i = 0; i < mapA->count; i++) {
                bool found = false;
                // Searching in MapB with MapA's key at index
                // we also do the key equality check
                PValue valB = MapObjGetValue(
                    (PObj *)b, mapA->table[i].vkey, mapA->table[i].key, &found
                );
                PValue valA = mapA->table[i].value;

                if (!found || !internalIsValueEqual(valA, valB, pair)) {
                    result = false;
                    break;
                }
            }

            SeenPairExit(pair, a, b);
            return result;
        }
    }

    return false;
}
