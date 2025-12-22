#include "../external/stb/stb_ds.h"
#include "../include/interpreter.h"
#include "../include/pstdlib.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

// TODO: check if index is too big to be a double
static finline u64 getArrIndex(PObj *arr, PValue val, bool *found) {
    u64 count = arr->v.OArray.count;
    for (u64 i = 0; i < count; i++) {
        if (IsValueEqual(val, arr->v.OArray.items[i])) {
            *found = true;
            return i;
        }
    }

    *found = false;
    return UINT64_MAX;
}

static PValue array_Exists(PInterpreter *it, PValue *args, u64 argc) {
    PValue rawArray = args[0];
    if (!IsValueObjType(rawArray, OT_ARR)) {
        return MakeError(it->gc, "Exists only takes array");
    }

    PValue key = args[1];
    bool found = false;
    (void)getArrIndex(ValueAsObj(rawArray), key, &found);
    return MakeBool(found);
}

static PValue array_Index(PInterpreter *it, PValue *args, u64 argc) {
    PValue rawArray = args[0];
    if (!IsValueObjType(rawArray, OT_ARR)) {
        return MakeError(it->gc, "Index only takes array");
    }

    PValue key = args[1];
    bool found = false;
    u64 index = getArrIndex(ValueAsObj(rawArray), key, &found);
    if (found) {
        return MakeNumber((double)index);
    } else {
        return MakeNumber(-1.0);
    }
}

static PValue array_Delete(PInterpreter *it, PValue *args, u64 argc) {
    if (argc != 1 && argc != 2) {
        return MakeError(
            it->gc, "pop functions can take either 1 or two arguments"
        );
    }

    PValue rawArray = args[0];
    if (!IsValueObjType(rawArray, OT_ARR)) {
        return MakeError(it->gc, "delete only works on arrays");
    }
    bool hasIndex = false;
    double rawIndex = 0.0;
    if (argc == 2) {
        hasIndex = true;
        if (IsValueNum(args[1])) {
            rawIndex = ValueAsNum(args[1]);
        } else {
            return MakeError(
                it->gc, "delete(array, index) -> index must be a number"
            );
        }
    }

    struct OArray *arr = &ValueAsObj(rawArray)->v.OArray;

    if (hasIndex) {
        if (rawIndex < 0) {
            return MakeError(
                it->gc,
                "delete(array, index) -> index must be a non-negetive integer"
            );
        }

        if (!IsDoubleInt(rawIndex)) {
            return MakeError(
                it->gc,
                "delete(array, index) -> index must be a non-negetive integer"
            );
        }

        u64 index = (u64)floor(rawIndex);

        if (index >= arr->count) {
            return MakeError(it->gc, "delete(....) index out of range");
        }

        PValue result = arr->items[index];
        arrdel(arr->items, index);
        return result;
    } else {
        if (arr->count == 0) {
            return MakeNil(); // empty array
        }

        PValue result = arrpop(arr->items);
        arr->count = arrlen(arr->items);

        return result;
    }
}

#define ARRAY_STD_EXISTS "exists"
#define ARRAY_STD_INDEX  "index"
#define ARRAY_STD_DELETE "delete"

void PushStdlibArray(PInterpreter *it, PEnv *env) {
    StdlibEntry entries[] = {
        MakeStdlibEntry(ARRAY_STD_EXISTS, array_Exists, 2),
        MakeStdlibEntry(ARRAY_STD_INDEX, array_Index, 2),
        MakeStdlibEntry(ARRAY_STD_DELETE, array_Delete, -1),
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
