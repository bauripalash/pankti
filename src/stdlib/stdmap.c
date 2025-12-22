#include "../external/stb/stb_ds.h"
#include "../include/env.h"
#include "../include/interpreter.h"
#include "../include/pstdlib.h"
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

static PValue map_Exists(PInterpreter *it, PValue *args, u64 argc) {
    PValue rawMap = args[0];

    if (!IsValueObjType(rawMap, OT_MAP)) {
        return MakeError(it->gc, "exists(...) only works with maps");
    }

    PObj *map = ValueAsObj(rawMap);
    PValue key = args[1];
    u64 keyHash = GetValueHash(key, it->gc->timestamp);
    bool hasKey = MapObjHasKey(map, key, keyHash);

    return MakeBool(hasKey);
}
static PValue map_Keys(PInterpreter *it, PValue *args, u64 argc) {
    PValue rawMap = args[0];

    if (!IsValueObjType(rawMap, OT_MAP)) {
        return MakeError(it->gc, "keys(...) only works with maps");
    }
    u64 count = 0;
    PValue *items = mapItemsAsArray(ValueAsObj(rawMap), &count, true);
    PObj *arrObj = NewArrayObject(it->gc, NULL, items, count);
    if (arrObj == NULL) {
        return MakeError(
            it->gc, "Internal Error : Failed to Create Keys Array"
        );
    }
    return MakeObject(arrObj);
}
static PValue map_Values(PInterpreter *it, PValue *args, u64 argc) {
    PValue rawMap = args[0];

    if (!IsValueObjType(rawMap, OT_MAP)) {
        return MakeError(it->gc, "values(...) only works with maps");
    }
    u64 count = 0;
    PValue *items = mapItemsAsArray(ValueAsObj(rawMap), &count, false);
    PObj *arrObj = NewArrayObject(it->gc, NULL, items, count);
    if (arrObj == NULL) {
        return MakeError(
            it->gc, "Internal Error : Failed to Create Values Array"
        );
    }
    return MakeObject(arrObj);
}

#define MAP_STD_EXISTS "বর্তমান"
#define MAP_STD_KEYS   "সূচক"
#define MAP_STD_VALUES "মান"

void PushStdlibMap(PInterpreter *it, PEnv *env) {
    StdlibEntry entries[] = {
        MakeStdlibEntry(MAP_STD_EXISTS, map_Exists, 2),
        MakeStdlibEntry(MAP_STD_KEYS, map_Keys, 1),
        MakeStdlibEntry(MAP_STD_VALUES, map_Values, 1),
    };
    int count = ArrCount(entries);
    for (int i = 0; i < count; i++) {
        const StdlibEntry *entry = &entries[i];
        PObj *stdFn = NewNativeFnObject(it->gc, NULL, entry->fn, entry->arity);
        EnvPutValue(
            env, StrHash(entry->name, entry->nlen, it->gc->timestamp),
            MakeObject(stdFn)
        );
    }
}
