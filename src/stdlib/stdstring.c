#include "../external/stb/stb_ds.h"
#include "../include/gc.h"
#include "../include/pstdlib.h"
#include "../include/unicode.h"
#include "../include/vm.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

static PValue str_Index(PVm *vm, PValue *args, u64 argc) {
    PValue rawStr = args[0];
    if (!IsValueObjType(rawStr, OT_STR)) {
        VmError(vm, RT_STDSTR_INDEX_STR_NOT_STRING, ValueTypeToStr(rawStr));
        return MakeNil();
    }

    PValue rawIndex = args[1];

    if (!IsValueNum(rawIndex)) {
        VmError(vm, RT_STDSTR_INDEX_INVALID_IDX_TYPE, ValueTypeToStr(rawIndex));
        return MakeNil();
    }
    double dblIndex = ValueAsNum(rawIndex);
    if (dblIndex < 0 || !IsDoubleInt(dblIndex)) {
        VmError(vm, RT_STDSTR_INDEX_INVALID_IDX_NUMTYPE, dblIndex);
        return MakeNil();
    }
    u64 index = (u64)floor(dblIndex);

    struct OString *str = &ValueAsObj(rawStr)->v.OString;

    GraphemeError err = GR_ERR_OK;
    u64 len = StrLength(str->value);
    char *result = GetGraphemeAt(str->value, len, index, &err);

    switch (err) {
        case GR_ERR_INDEX_OUT_RANGE: {
            u64 graphemeCount = GetGraphemeCount(str->value, len);
            u64 maxIdx = graphemeCount == 0 ? 0 : graphemeCount - 1;
            VmError(vm, RT_STDSTR_INDEX_INDEX_OUT_RANGE, maxIdx);
            return MakeNil();
        }; // error
        case GR_ERR_MEM: {
            VmError(vm, RT_IME_STDSTR_INDEX_GRAPHEME_MEM);
            return MakeNil();
        }; // error
        case GR_ERR_EMPTY: {
            char *emptyStr = StrDuplicate("", 0);
            PObj *obj = NewStrObject(vm->gc, NULL, emptyStr, false);
            if (obj == NULL) {
                VmError(vm, RT_IME_STDSTR_INDEX_RESULT_STR);
                return MakeNil();
            }
            return MakeObject(obj);
        }; // empty string
        case GR_ERR_OK: {
            PObj *obj = NewStrObject(vm->gc, NULL, result, false);
            if (obj == NULL) {
                VmError(vm, RT_IME_STDSTR_INDEX_RESULT_STR);
                return MakeNil();
            }
            return MakeObject(obj);
        }
    }
    VmError(vm, RT_IME_STDSTR_INDEX_UNREACHABLE);
    return MakeNil(); // should never reach here
}

static PValue str_Split(PVm *vm, PValue *args, u64 argc) {
    PValue rawStr = args[0];
    PValue rawDelim = args[1];
    if (!IsValueObjType(rawStr, OT_STR)) {
        VmError(vm, RT_STDSTR_SPLIT_STR_NOT_STRING, ValueTypeToStr(rawStr));
        return MakeNil();
    }
    if (!IsValueObjType(rawDelim, OT_STR)) {
        VmError(vm, RT_STDSTR_SPLIT_DELIM_NOT_STRING, ValueTypeToStr(rawDelim));
        return MakeNil();
    }

    struct OString *str = &ValueAsObj(rawStr)->v.OString;
    struct OString *delim = &ValueAsObj(rawDelim)->v.OString;

    bool isok = true;
    u64 count = 0;
    char **result = StrSplitDelim(str->value, delim->value, &count, &isok);
    if (!isok) {
        VmError(vm, RT_IME_STDSTR_SPLIT_MEM);
        return MakeNil();
    }

    PValue *items = NULL;
    for (u64 i = 0; i < count; i++) {
        PObj *tempStr = NewStrObject(vm->gc, NULL, result[i], false);
        if (tempStr == NULL) {
            VmError(vm, RT_IME_STDSTR_SPLIT_TEMPSTR);
            return MakeNil();
            // we just return here, the orpahned objects will handled by the gc
        }

        arrput(items, MakeObject(tempStr));
    }

    PObj *arr = NewArrayObject(vm->gc, NULL, items, count);
    if (arr == NULL) {
        VmError(vm, RT_IME_STDSTR_SPLIT_RESULTARR);
        return MakeNil();
    }
    arrfree(result);
    // we don't need to free the substrings, arr items now own them
    return MakeObject(arr);
}

static PValue str_String(PVm *vm, PValue *args, u64 argc) {
    PValue target = args[0];
    char *str = ValueToString(target); // we don't need to free this
    // as ownership is given to string object crated below
    if (str == NULL) {
        VmError(vm, RT_IME_STDSTR_STR_VALTOSTR, ValueTypeToStr(target));
        return MakeNil(); // error
    }

    PObj *strObj = NewStrObject(vm->gc, NULL, str, true);
    if (strObj == NULL) {
        VmError(vm, RT_IME_STDSTR_STR_RESULT, ValueTypeToStr(target));
        return MakeNil();
    }
    return MakeObject(strObj);
}

#define STR_STD_INDEX  "সূচক"
#define STR_STD_SPLIT  "ভাগ"
#define STR_STD_STRING "পরিবর্তন"

void PushStdlibString(PVm *vm, SymbolTable *table) {
    StdlibEntry entries[] = {
        MakeStdlibEntry(STR_STD_INDEX, str_Index, 2),
        MakeStdlibEntry(STR_STD_SPLIT, str_Split, 2),
        MakeStdlibEntry(STR_STD_STRING, str_String, 1)
    };

    int count = ArrCount(entries);
    PushStdlibEntries(vm, table, STRING_STDLIB_NAME, entries, count);
}
