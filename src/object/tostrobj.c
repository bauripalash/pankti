/*
 * Copyright (c) 2022 Palash Bauri
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>

#include "../external/gb/gb_string.h"
#include "../include/alloc.h"
#include "../include/flags.h"
#include "../include/object.h"
#include "../include/panktiterms.h"
#include "../include/utils.h"

static char *internalValueToString(PValue val, ObjSeenSet *set);
static char *internalObjToString(const PObj *obj, ObjSeenSet *set);

char *ValueToString(PValue val) {
    ObjSeenSet set = {0};
    return internalValueToString(val, &set);
}

char *ObjToString(PObj *obj) {
    if (obj == NULL) {
        return NULL;
    }
    ObjSeenSet set = {0};
    return internalObjToString(obj, &set);
}

char *internalValueToString(PValue val, ObjSeenSet *seen) {
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
        result = internalObjToString(ValueAsObj(val), seen);
    }

    return result;
}

static char *internalObjToString(const PObj *obj, ObjSeenSet *seen) {
    char *result = NULL;
    switch (obj->type) {
            // Seen Set Safe
        case OT_STR: {
            const struct OString *str = &obj->v.OString;
            if (str->value == NULL) {
                return NULL;
            }

            result = StrDuplicate(str->value, StrLength(str->value));
            break;
        }
        case OT_COMFNC: {
            const struct OComFunction *fn = &obj->v.OComFunction;
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
            const struct OClosure *cls = &obj->v.OClosure;
            result = internalObjToString(cls->function, seen);
            break;
        }

        case OT_NATIVE: {
            const struct ONative *nfn = &obj->v.ONative;
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
            // Needs Seen Guard
        case OT_ARR: {
            if (SeenSetEnter(seen, obj)) {
                return StrDuplicate("[...]", 5);
            }
            const struct OArray *arr = &obj->v.OArray;

            gbString s = gb_make_string("[");

            for (u64 i = 0; i < arr->count; i++) {
                PValue val = arr->items[i];
                char *temp = internalValueToString(val, seen);
                s = gb_append_cstring(s, temp);

                if (i != arr->count - 1) {
                    s = gb_append_cstring(s, ", ");
                }
                PFree(temp);
            }
            s = gb_append_cstring(s, "]");
            result = StrDuplicate(s, (u64)gb_string_length(s));
            gb_free_string(s);
            SeenSetExit(seen, obj);
            break;
        }
        case OT_MAP: {
            if (SeenSetEnter(seen, obj)) {
                return StrDuplicate("{...}", 5);
            }
            const struct OMap *map = &obj->v.OMap;
            gbString s = gb_make_string("{");
            u64 count = map->count;
            for (u64 i = 0; i < count; i++) {
                PValue key = map->table[i].vkey;
                PValue val = map->table[i].value;

                char *keyStr = internalValueToString(key, seen);
                char *valStr = internalValueToString(val, seen);

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
            SeenSetExit(seen, obj);
            break;
        }
    }

    return result;
}
