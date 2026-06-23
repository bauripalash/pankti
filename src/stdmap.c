/*
 * Copyright (c) 2022 Palash Bauri
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "external/stb/stb_ds.h"
#include "gc.h"
#include "pstdlib.h"
#include "vm.h"
#include <stdbool.h>

// Fetch map items as array, if needKeys is true returns the keys list
// if needKeys is false returns value list
static PValue *mapItemsAsArray(PObj *map, u64 *count, bool needKeys) {
    PValue *arr = NULL;
    struct OMap *m = &map->v.OMap;
    u64 len = hmlen(m->table);

    for (u64 i = 0; i < len; i++) {
        if (needKeys) {
            arrput(arr, m->table[i].vkey);
        } else {
            arrput(arr, m->table[i].value);
        }
    }

    *count = len;
    return arr;
}

static PValue map_Exists(PVm *vm, PValue *args, u64 argc) {
    PValue rawMap = args[0];

    if (!IsValueObjType(rawMap, OT_MAP)) {
        VmError(vm, RT_STDMAP_EXISTS_FIRST_NOTMAP, ValueTypeToStr(rawMap));
        return MakeNil();
    }

    PObj *map = ValueAsObj(rawMap);
    PValue key = args[1];

    if (!CanValueBeKey(key)) {
        VmError(vm, RT_STDMAP_EXISTS_INVALID_KEY, ValueTypeToStr(key));
        return MakeNil();
    }

    u64 keyHash = GetValueHash(key, vm->gc->timestamp);
    bool hasKey = MapObjHasKey(map, key, keyHash);

    return MakeBool(hasKey);
}
static PValue map_Keys(PVm *vm, PValue *args, u64 argc) {
    PValue rawMap = args[0];

    if (!IsValueObjType(rawMap, OT_MAP)) {
        VmError(vm, RT_STDMAP_KEYS_FIRST_NOTMAP, ValueTypeToStr(rawMap));
        return MakeNil();
    }
    u64 count = 0;
    PValue *items = mapItemsAsArray(ValueAsObj(rawMap), &count, true);
    PObj *arrObj = NewArrayObject(vm->gc, NULL, items, count);
    if (arrObj == NULL) {
        VmError(vm, RT_IME_STDMAP_KEYS_ARR_CREATE);

        return MakeNil();
    }
    return MakeObject(arrObj);
}
static PValue map_Values(PVm *vm, PValue *args, u64 argc) {
    PValue rawMap = args[0];

    if (!IsValueObjType(rawMap, OT_MAP)) {
        VmError(vm, RT_STDMAP_VALUES_FIRST_NOTMAP, ValueTypeToStr(rawMap));
        return MakeNil();
    }
    u64 count = 0;
    PValue *items = mapItemsAsArray(ValueAsObj(rawMap), &count, false);
    PObj *arrObj = NewArrayObject(vm->gc, NULL, items, count);
    if (arrObj == NULL) {
        VmError(vm, RT_IME_STDMAP_VALUES_ARR_CREATE);
        return MakeNil();
    }
    return MakeObject(arrObj);
}

static PValue map_Delete(PVm *vm, PValue *args, u64 argc) {
    PValue rawMap = args[0];
    if (!IsValueObjType(rawMap, OT_MAP)) {
        VmError(vm, RT_STDMAP_DELETE_FIRST_NOTMAP, ValueTypeToStr(rawMap));
        return MakeNil();
    }
    PValue key = args[1];
    if (!CanValueBeKey(key)) {
        VmError(vm, RT_STDMAP_DELETE_INVALID_KEY, ValueTypeToStr(key));
        return MakeNil();
    }
    u64 keyHash = GetValueHash(key, vm->gc->timestamp);
    PObj *map = ValueAsObj(rawMap);
    bool ok = false;
    PValue result = MapObjRemoveKey(map, key, keyHash, &ok);
    if (!ok) {
        VmError(vm, RT_STDMAP_DELETE_KEY_NOTFOUND);
        return MakeNil();
    }
    return result;
}

#define MAP_STD_EXISTS "বর্তমান"
#define MAP_STD_KEYS   "সূচকগুলি"
#define MAP_STD_VALUES "মানগুলি"
#define MAP_STD_DELETE "বিয়োগ"

void PushStdlibMap(PVm *vm, SymbolTable *table) {
    StdlibEntry entries[] = {
        MakeStdlibEntry(MAP_STD_EXISTS, map_Exists, 2),
        MakeStdlibEntry(MAP_STD_KEYS, map_Keys, 1),
        MakeStdlibEntry(MAP_STD_VALUES, map_Values, 1),
        MakeStdlibEntry(MAP_STD_DELETE, map_Delete, 2),
    };
    int count = ArrCount(entries);

    PushStdlibEntries(vm, table, MAP_STDLIB_NAME, entries, count);
}
