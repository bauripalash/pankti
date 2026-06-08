/*
 * Copyright (c) 2022 Palash Bauri
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "../external/stb/stb_ds.h"
#include "../external/xxhash/xxhash.h"
#include "../include/object.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

// Golden ratio?
#define CONST_NAN_HASH 0x9e3779b97f4a7c15ULL
// MurmurHash3
#define CONST_ZERO_HASH       0xff51afd7ed558ccdULL

#define CONST_BOOL_TRUE_HASH  0x1
#define CONST_BOOL_FALSE_HASH 0x0
#define CONST_NIL_HASH        0x2

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
