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

// Check if value as arg1 exists in array as arg0
static PValue array_Exists(PVm *vm, PValue *args, u64 argc) {
    PValue rawArray = args[0];
    if (!IsValueObjType(rawArray, OT_ARR)) {
        VmError(vm, RT_STDARR_EXISTS_FIRST_ARR, ValueTypeToStr(rawArray));
        return MakeNil();
    }

    PValue key = args[1];
    bool found = false;
    (void)getArrIndex(ValueAsObj(rawArray), key, &found);
    return MakeBool(found);
}

static PValue array_Add(PVm *vm, PValue *args, u64 argc) {
    PValue rawArray = args[0];
    if (!IsValueObjType(rawArray, OT_ARR)) {
        VmError(vm, RT_STDARR_ADD_FIRST_ARR, ValueTypeToStr(rawArray));
        return MakeNil();
    }

    PValue rawIndex = args[1];

    if (!IsValueNum(rawIndex)) {
        VmError(vm, RT_STDARR_ADD_INVALID_IDX_TYPE, ValueTypeToStr(rawIndex));
        return MakeNil();
    }

    double dblIndex = ValueAsNum(rawIndex);

    if (dblIndex < 0 || !IsDoubleInt(dblIndex)) {
        VmError(vm, RT_STDARR_ADD_INVALID_NUMIDX, dblIndex);
        return MakeNil();
    }

    u64 arrIndex = (u64)floor(dblIndex);
    struct OArray *arr = &ValueAsObj(rawArray)->v.OArray;
    PValue val = args[2];

    if (arrIndex >= arr->count) {
        u64 indexLimit = arr->count == 0 ? 0 : arr->count - 1;
        VmError(vm, RT_STDARR_ADD_INDEX_OUT_RANGE, indexLimit);
        return MakeNil();
    }

    if (arr->items == NULL) {
        // if reached here it means the array count and array items are either
        // out of sync, or somehow items array is corrupted.
        VmError(vm, RT_IME_STDARR_ADD_ARRAY_ITEMS_OUTSYNC);
        return MakeNil();
    }

    arrins(arr->items, arrIndex, val);
    arr->count = arrlen(arr->items);
    return MakeNumber((double)arr->count);
}

static PValue array_Index(PVm *vm, PValue *args, u64 argc) {
    PValue rawArray = args[0];
    if (!IsValueObjType(rawArray, OT_ARR)) {
        VmError(vm, RT_STDARR_INDEX_FIRST_ARR, ValueTypeToStr(rawArray));
        return MakeNil();
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
    PValue rawArray = args[0];
    if (!IsValueObjType(rawArray, OT_ARR)) {
        VmError(vm, RT_STDARR_DELETE_FIRST_ARR, ValueTypeToStr(rawArray));
        return MakeNil();
    }

    PValue rawIndex = args[1];
    if (!IsValueNum(rawIndex)) {
        VmError(
            vm, RT_STDARR_DELETE_INVALID_IDX_TYPE, ValueTypeToStr(rawIndex)
        );
        return MakeNil();
    }

    double dblIndex = ValueAsNum(rawIndex);
    if (dblIndex < 0 || !IsDoubleInt(dblIndex)) {
        VmError(vm, RT_STDARR_DELETE_INVALID_NUMIDX, dblIndex);
        return MakeNil();
    }

    u64 arrIndex = (u64)floor(dblIndex);
    struct OArray *arr = &ValueAsObj(rawArray)->v.OArray;

    if (arrIndex >= arr->count) {
        u64 indexLimit = arr->count == 0 ? 0 : arr->count - 1;
        VmError(vm, RT_STDARR_DELETE_INDEX_OUT_RANGE, indexLimit);
        return MakeNil();
    }

    if (arr->items == NULL) {
        VmError(vm, RT_IME_STDARR_DELETE_ARRAY_ITEMS_OUTSYNC);
        return MakeNil();
    }

    PValue result = arr->items[arrIndex];
    arrdel(arr->items, arrIndex);
    arr->count = arrlen(arr->items);
    return result;
}

static PValue array_Trim(PVm *vm, PValue *args, u64 argc) {
    PValue rawArray = args[0];
    if (!IsValueObjType(rawArray, OT_ARR)) {
        VmError(vm, RT_STDARR_TRIM_FIRST_ARR, ValueTypeToStr(rawArray));
        return MakeNil();
    }

    struct OArray *arr = &ValueAsObj(rawArray)->v.OArray;
    if (arr->count == 0) {
        VmError(vm, RT_STDARR_TRIM_ARR_EMPTY);
        return MakeNil();
    }

    if (arr->items == NULL) {
        VmError(vm, RT_IME_STDARR_TRIM_ARRAY_ITEMS_OUTSYNC);
        return MakeNil();
    }

    return arrpop(arr->items);
}

#define ARRAY_STD_EXISTS "বর্তমান"
#define ARRAY_STD_INDEX  "সূচক"
#define ARRAY_STD_ADD    "সংযোগ"
#define ARRAY_STD_DELETE "বিয়োগ"
#define ARRAY_STD_TRIM   "কাটো"

void PushStdlibArray(PVm *vm, SymbolTable *table) {
    StdlibEntry entries[] = {
        MakeStdlibEntry(ARRAY_STD_EXISTS, array_Exists, 2),
        MakeStdlibEntry(ARRAY_STD_INDEX, array_Index, 2),
        MakeStdlibEntry(ARRAY_STD_ADD, array_Add, 3),
        MakeStdlibEntry(ARRAY_STD_DELETE, array_Delete, 2),
        MakeStdlibEntry(ARRAY_STD_TRIM, array_Trim, 1),
    };

    int count = ArrCount(entries);

    PushStdlibEntries(vm, table, ARRAY_STDLIB_NAME, entries, count);
}
