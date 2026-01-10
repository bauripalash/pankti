#include "../external/stb/stb_ds.h"
#include "../include/gc.h"
#include "../include/pstdlib.h"
#include "../include/unicode.h"
#include "../include/vm.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

static PValue str_Index(PVm *vm, PValue *args, u64 argc) {
    PValue rawStr = args[0];
    if (!IsValueObjType(rawStr, OT_STR)) {
        return MakeError(
            vm->gc, "Index(...) function's first argument must be a string"
        );
    }

    PValue rawIndex = args[1];
    u64 index = 0;
    if (!IsValueNum(rawIndex)) {

        return MakeError(
            vm->gc, "Index(...) index argument must be a non-negetive intger"
        );
    } else {
        double dIndex = ValueAsNum(rawIndex);
        if (!IsDoubleInt(dIndex)) {
            return MakeError(
                vm->gc,
                "Index(...) index argument must be a non-negetive intger"
            );
        }

        index = (u64)floor(dIndex);
    }

    struct OString *str = &ValueAsObj(rawStr)->v.OString;

    GraphemeError err = GR_ERR_OK;
    u64 len = StrLength(str->value);
    char *result = GetGraphemeAt(str->value, len, index, &err);

    switch (err) {
        case GR_ERR_INDEX_OUT_RANGE: {
            return MakeError(vm->gc, "Index(...) index is out of range");
        }; // error
        case GR_ERR_MEM: {
            return MakeError(
                vm->gc, "Internal Error : Index(...) failed due to memory error"
            );
        }; // error
        case GR_ERR_EMPTY: {
            return MakeNil(); // return empty string
        }; // empty string
        case GR_ERR_OK: {
            PObj *obj = NewStrObject(vm->gc, NULL, result, true);
            // todo: null check
            return MakeObject(obj);
        }
    }

    return MakeNil(); // should never reach here
}

static PValue str_Split(PVm *vm, PValue *args, u64 argc) {
    PValue rawStr = args[0];
    PValue rawDelim = args[1];
    if (!IsValueObjType(rawStr, OT_STR) || !IsValueObjType(rawDelim, OT_STR)) {
        return MakeError(
            vm->gc, "Split(...) function's both arguments must be string"
        );
    }

    struct OString *str = &ValueAsObj(rawStr)->v.OString;
    struct OString *delim = &ValueAsObj(rawDelim)->v.OString;

    bool isok = true;
    u64 count = 0;
    char **result = StrSplitDelim(str->value, delim->value, &count, &isok);

    if (isok) {

        PValue *items = NULL;
        for (u64 i = 0; i < count; i++) {
            PObj *tempStr = NewStrObject(vm->gc, NULL, result[i], true);
            if (tempStr == NULL) {
                return MakeNil(); // memory error //todo
            }

            arrput(items, MakeObject(tempStr));
        }

        PObj *arr = NewArrayObject(vm->gc, NULL, items, count);
        arrfree(
            result
        ); // we don't need to free the substrings, arr items now own them
        return MakeObject(arr);

    } else {
        return MakeError(
            vm->gc, "Internal Error : Split(...) failed due to memory error"
        );
    }

    return MakeNil(); // should never reach here
}

static PValue str_String(PVm *vm, PValue *args, u64 argc) {
    PValue target = args[0];
    char *str = ValueToString(target); // we don't need to free this
    // as ownership is given to string object crated below
    if (str == NULL) {
        return MakeNil(); // error
    }

    PObj *strObj = NewStrObject(vm->gc, NULL, str, true);
    return MakeObject(strObj);
}

#define STR_STD_INDEX  "index"
#define STR_STD_SPLIT  "split"
#define STR_STD_STRING "string"

void PushStdlibString(PVm *vm, SymbolTable *table) {
    StdlibEntry entries[] = {
        MakeStdlibEntry(STR_STD_INDEX, str_Index, 2),
        MakeStdlibEntry(STR_STD_SPLIT, str_Split, 2),
        MakeStdlibEntry(STR_STD_STRING, str_String, 1)
    };

    int count = ArrCount(entries);
    PushStdlibEntries(vm, table, entries, count);
}
