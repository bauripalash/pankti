#include <stdio.h>

#include "../external/gb/gb_string.h"
#include "../include/alloc.h"
#include "../include/flags.h"
#include "../include/object.h"
#include "../include/panktiterms.h"
#include "../include/utils.h"

char *ValueToString(PValue val) {
    char *result = NULL;
    if (IsValueNum(val)) {
        double dblNum = ValueAsNum(val);

        char enBuf[NUM_STR_BUF_SIZE];

        if (IsDoubleInt(dblNum)) {
            snprintf(enBuf, NUM_STR_BUF_SIZE, "%0.f", dblNum);
        } else {
            FormatDouble(dblNum, enBuf, NUM_STR_BUF_SIZE);
        }

#if defined(PANKTI_BUILD_DEBUG)
        if (FLAG_ENGLISH_NUM) {
            result = StrDuplicate(enBuf, StrLength(enBuf));
            return result;
        } else {
            char bnBuf[BN_NUM_STR_BUF_SIZE];
            NumberStrToBnStr(enBuf, bnBuf, BN_NUM_STR_BUF_SIZE);
            result = StrDuplicate(bnBuf, StrLength(bnBuf));
            return result;
        }
#else
        char bnBuf[BN_NUM_STR_BUF_SIZE];
        NumberStrToBnStr(enBuf, bnBuf, BN_NUM_STR_BUF_SIZE);
        result = StrDuplicate(bnBuf, StrLength(bnBuf));
        return result;
#endif

    } else if (IsValueBool(val)) {
        if (ValueAsBool(val) == true) { // just make it more verbose
            result = StrDuplicate(PANTERM_TRUE, StrLength(PANTERM_TRUE));
        } else {
            result = StrDuplicate(PANTERM_FALSE, StrLength(PANTERM_FALSE));
        }
    } else if (IsValueNil(val)) {
        result = StrDuplicate(PANTERM_NIL, StrLength(PANTERM_NIL));
    } else if (IsValueObj(val)) {
        result = ObjToString(ValueAsObj(val));
    }

    return result;
}

char *ObjToString(PObj *obj) {
    char *result = NULL;
    switch (obj->type) {
        case OT_STR: {
            struct OString *str = &obj->v.OString;
            if (str->value == NULL) {
                return NULL;
            }

            result = StrDuplicate(str->value, StrLength(str->value));
            break;
        }
        case OT_COMFNC: {
            struct OComFunction *fn = &obj->v.OComFunction;
            if (fn->strName != NULL) {
                const char *temp = StrFormat(
                    "<%s '%s'>", PANTERM_FUNCTION,
                    fn->strName != NULL ? fn->strName->v.OString.value
                                        : PANTERM_UNKNOWN
                );
                result = StrDuplicate(temp, StrLength(temp));
            } else {
                result = StrDuplicate(
                    "<" PANTERM_SCRIPT ">", StrLength("<" PANTERM_SCRIPT ">")
                );
            }

            break;
        }
        case OT_CLOSURE: {
            struct OClosure *cls = &obj->v.OClosure;
            result = ObjToString(cls->function);
            break;
        }
        case OT_ARR: {
            struct OArray *arr = &obj->v.OArray;

            gbString s = gb_make_string("[");

            for (u64 i = 0; i < arr->count; i++) {
                PValue val = arr->items[i];
                char *temp = ValueToString(val);
                s = gb_append_cstring(s, temp);

                if (i != arr->count - 1) {
                    s = gb_append_cstring(s, ", ");
                }
                PFree(temp);
            }
            s = gb_append_cstring(s, "]");
            result = StrDuplicate(s, (u64)gb_string_length(s));
            gb_free_string(s);
            break;
        }
        case OT_MAP: {
            struct OMap *map = &obj->v.OMap;
            gbString s = gb_make_string("{");
            u64 count = map->count;
            for (u64 i = 0; i < count; i++) {
                PValue key = map->table[i].vkey;
                PValue val = map->table[i].value;

                char *keyStr = ValueToString(key);
                char *valStr = ValueToString(val);

                if (keyStr == NULL || valStr == NULL) {
                    gb_free_string(s);
                    PFree(result);
                    result = NULL;

                    if (keyStr != NULL) {
                        PFree(keyStr);
                    }

                    if (valStr != NULL) {
                        PFree(valStr);
                    }

                    return NULL;
                }

                s = gb_append_cstring(s, StrFormat("%s: %s", keyStr, valStr));
                if (i != count - 1) {
                    s = gb_append_cstring(s, ", ");
                }
                PFree(keyStr);
                PFree(valStr);
            }

            s = gb_append_cstring(s, "}");
            result = StrDuplicate(s, (u64)gb_string_length(s));
            gb_free_string(s);
            break;
        }
        case OT_NATIVE: {
            struct ONative *nfn = &obj->v.ONative;
            const char *temp = StrFormat(
                "<%s '%s'>", PANTERM_NATIVE_FUNC,
                nfn->name != NULL ? nfn->name : PANTERM_UNKNOWN
            );
            result = StrDuplicate(temp, strlen(temp));
            break;
        }
        case OT_MODULE: {
            const char *name = NULL;
            bool hasName = false;
            if (obj->v.OModule.customName != NULL) {
                name = obj->v.OModule.customName;
                hasName = true;
            }
            const char *temp = StrFormat(
                "<%s '%s'>", PANTERM_MODULE, hasName ? name : PANTERM_UNKNOWN
            );
            result = StrDuplicate(temp, StrLength(temp));
            break;
        }
        case OT_UPVAL: {
            const char *temp = StrFormat("<" PANTERM_UPVALUE ">");
            result = StrDuplicate(temp, strlen(temp));
            break;
        }
    }

    return result;
}
