#include "../external/stb/stb_ds.h"
#include "../include/gc.h"
#include "../include/pstdlib.h"
#include "../include/vm.h"
#include <stdbool.h>
#include <stdlib.h>

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
        return MakeError(vm->gc, "exists(...) only works with maps");
    }

    PObj *map = ValueAsObj(rawMap);
    PValue key = args[1];
    u64 keyHash = GetValueHash(key, vm->gc->timestamp);
    bool hasKey = MapObjHasKey(map, key, keyHash);

    return MakeBool(hasKey);
}
static PValue map_Keys(PVm *vm, PValue *args, u64 argc) {
    PValue rawMap = args[0];

    if (!IsValueObjType(rawMap, OT_MAP)) {
        return MakeError(vm->gc, "keys(...) only works with maps");
    }
    u64 count = 0;
    PValue *items = mapItemsAsArray(ValueAsObj(rawMap), &count, true);
    PObj *arrObj = NewArrayObject(vm->gc, NULL, items, count);
    if (arrObj == NULL) {
        return MakeError(
            vm->gc, "Internal Error : Failed to Create Keys Array"
        );
    }
    return MakeObject(arrObj);
}
static PValue map_Values(PVm *vm, PValue *args, u64 argc) {
    PValue rawMap = args[0];

    if (!IsValueObjType(rawMap, OT_MAP)) {
        return MakeError(vm->gc, "values(...) only works with maps");
    }
    u64 count = 0;
    PValue *items = mapItemsAsArray(ValueAsObj(rawMap), &count, false);
    PObj *arrObj = NewArrayObject(vm->gc, NULL, items, count);
    if (arrObj == NULL) {
        return MakeError(
            vm->gc, "Internal Error : Failed to Create Values Array"
        );
    }
    return MakeObject(arrObj);
}

#define MAP_STD_EXISTS "বর্তমান"
#define MAP_STD_KEYS   "সূচক"
#define MAP_STD_VALUES "মান"

void PushStdlibMap(PVm *vm, SymbolTable *table) {
    StdlibEntry entries[] = {
        MakeStdlibEntry(MAP_STD_EXISTS, map_Exists, 2),
        MakeStdlibEntry(MAP_STD_KEYS, map_Keys, 1),
        MakeStdlibEntry(MAP_STD_VALUES, map_Values, 1),
    };
    int count = ArrCount(entries);

    PushStdlibEntries(vm, table, entries, count);
}
