#include "../external/stb/stb_ds.h"
#include "../include/gc.h"
#include "../include/pstdlib.h"
#include "../include/vm.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

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

static PValue array_Exists(PVm *vm, PValue *args, u64 argc) {
    PValue rawArray = args[0];
    if (!IsValueObjType(rawArray, OT_ARR)) {
        return MakeError(vm->gc, "Exists only takes array");
    }

    PValue key = args[1];
    bool found = false;
    (void)getArrIndex(ValueAsObj(rawArray), key, &found);
    return MakeBool(found);
}

static PValue array_Add(PVm *vm, PValue *args, u64 argc) {

    PValue rawArray = args[0];
    if (!IsValueObjType(rawArray, OT_ARR)) {
        return MakeError(vm->gc, "add only takes array");
    }

    PValue rawIndex = args[1];
    double dblIndex = -1;
    if (IsValueNum(rawIndex)) {
        dblIndex = ValueAsNum(rawIndex);

        if (dblIndex < 0) {
            return MakeError(
                vm->gc, "add(array, index, value) -> index must be a "
                        "non-negetive integer"
            );
        }

        if (!IsDoubleInt(dblIndex)) {
            return MakeError(
                vm->gc, "add(array, index, value) -> index must be a "
                        "non-negetive integer"
            );
        }
    } else {
        return MakeError(
            vm->gc,
            "add(array, index, value) -> index must be a non-negetive integer"
        );
    }
    u64 arrIndex = (u64)floor(dblIndex);
    struct OArray *arr = &ValueAsObj(rawArray)->v.OArray;

    if (arrIndex >= arr->count) {
        return MakeError(vm->gc, "add(....) index out of range");
    }

    if (arrIndex > 0 && arr->items == NULL) {
        // TODO: Error handle on NULL
        return MakeError(
            vm->gc, "Internal Error : Failed to add item at index `<TODO>` as "
                    "the array is null? <TODO>"
        );
    }

    PValue val = args[2];
    arrins(arr->items, arrIndex, val);
    arr->count = arrlen(arr->items);
    return MakeNumber((double)arr->count);
}

static PValue array_Index(PVm *vm, PValue *args, u64 argc) {
    PValue rawArray = args[0];
    if (!IsValueObjType(rawArray, OT_ARR)) {
        return MakeError(vm->gc, "Index only takes array");
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

static PValue array_Delete(PVm *vm, PValue *args, u64 argc) {
    if (argc != 1 && argc != 2) {
        return MakeError(
            vm->gc, "pop functions can take either 1 or two arguments"
        );
    }

    PValue rawArray = args[0];
    if (!IsValueObjType(rawArray, OT_ARR)) {
        return MakeError(vm->gc, "delete only works on arrays");
    }
    bool hasIndex = false;
    double rawIndex = 0.0;
    if (argc == 2) {
        hasIndex = true;
        if (IsValueNum(args[1])) {
            rawIndex = ValueAsNum(args[1]);
        } else {
            return MakeError(
                vm->gc, "delete(array, index) -> index must be a number"
            );
        }
    }

    struct OArray *arr = &ValueAsObj(rawArray)->v.OArray;

    if (hasIndex) {
        if (rawIndex < 0) {
            return MakeError(
                vm->gc,
                "delete(array, index) -> index must be a non-negetive integer"
            );
        }

        if (!IsDoubleInt(rawIndex)) {
            return MakeError(
                vm->gc,
                "delete(array, index) -> index must be a non-negetive integer"
            );
        }

        u64 index = (u64)floor(rawIndex);

        if (index >= arr->count) {
            return MakeError(vm->gc, "delete(....) index out of range");
        }

        // TODO: Null Check when null and index is greater than 0
        PValue result = arr->items[index];
        arrdel(arr->items, index);
        arr->count = arrlen(arr->items);
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

#define ARRAY_STD_EXISTS "বর্তমান"
#define ARRAY_STD_INDEX  "সূচক"
#define ARRAY_STD_ADD    "সংযোগ"
#define ARRAY_STD_DELETE "বিয়োগ"

void PushStdlibArray(PVm *vm, SymbolTable *table) {
    StdlibEntry entries[] = {
        MakeStdlibEntry(ARRAY_STD_EXISTS, array_Exists, 2),
        MakeStdlibEntry(ARRAY_STD_INDEX, array_Index, 2),
        MakeStdlibEntry(ARRAY_STD_ADD, array_Add, 3),
        MakeStdlibEntry(ARRAY_STD_DELETE, array_Delete, -1),
    };

    int count = ArrCount(entries);

    PushStdlibEntries(vm, table, entries, count);
}
