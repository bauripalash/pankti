#include "../include/flags.h"
#include "../include/object.h"
#include "../include/panktiterms.h"
#include "../include/printer.h"
#include "../include/utils.h"

#include "../external/stb/stb_ds.h"

#include <stdio.h>

static void internalPrintValue(PValue val, ObjSeenSet *seen);
static void internalPrintObj(const PObj *obj, ObjSeenSet *seen);

void PrintValue(PValue val) {
    ObjSeenSet seen = {0};
    internalPrintValue(val, &seen);
}

void PrintObject(const PObj *o) {
    if (o == NULL) {
        return;
    }
    ObjSeenSet seen = {0};
    internalPrintObj(o, &seen);
}

static void internalPrintValue(PValue val, ObjSeenSet *seen) {
    if (IsValueNum(val)) {
        double num = ValueAsNum(val);

        char enBuf[NUM_STR_BUF_SIZE];

        if (IsDoubleInt(num)) {
            snprintf(enBuf, NUM_STR_BUF_SIZE, "%0.f", num);
        } else {
            FormatDouble(num, enBuf, NUM_STR_BUF_SIZE);
        }

#if defined(PANKTI_BUILD_DEBUG)
        if (FLAG_ENGLISH_NUM) {
            PanPrint("%s", enBuf);
        } else {
            char bnBuf[BN_NUM_STR_BUF_SIZE];
            NumberStrToBnStr(enBuf, bnBuf, BN_NUM_STR_BUF_SIZE);
            PanPrint("%s", bnBuf);
        }
#else
        char bnBuf[BN_NUM_STR_BUF_SIZE];
        NumberStrToBnStr(enBuf, bnBuf, BN_NUM_STR_BUF_SIZE);
        PanPrint("%s", bnBuf);

#endif

    } else if (IsValueBool(val)) {
        PanPrint("%s", ValueAsBool(val) ? PANTERM_TRUE : PANTERM_FALSE);
    } else if (IsValueNil(val)) {
        PanPrint(PANTERM_NIL);
    } else if (IsValueObj(val)) {
        internalPrintObj(ValueAsObj(val), seen);
    } else {
        PanPrint(PANTERM_UNKNOWN);
    }
}

static void internalPrintObj(const PObj *o, ObjSeenSet *seen) {
    if (o == NULL) {
        return;
    }

    switch (o->type) {
            // Seen Guard Safe
        case OT_STR: {
            const struct OString *str = &o->v.OString;
            if (str->value != NULL) {
                PanPrint("%s", str->value);
            } else {
                PanPrint("<" PANTERM_UNKNOWN ">");
            }

            break;
        }
        case OT_COMFNC: {
            const struct OComFunction *func = &o->v.OComFunction;
            if (func->strName != NULL) {
                PanPrint(
                    "<%s '%s'>", PANTERM_FUNCTION,
                    func->strName->v.OString.value
                );
            } else {
                PanPrint("<%s>", PANTERM_SCRIPT);
            }
            break;
        }
        case OT_CLOSURE: {
            const struct OClosure *cls = &o->v.OClosure;
            internalPrintObj(cls->function, seen);
            break;
        }
        case OT_NATIVE: {
            PanPrint(
                "<%s '%s'>", PANTERM_NATIVE_FUNC,
                o->v.ONative.name != NULL ? o->v.ONative.name : PANTERM_UNKNOWN
            );
            break;
        }
        case OT_MODULE: {
            char *name = NULL;
            const struct OModule *mod = &o->v.OModule;
            if (mod->customName != NULL) {
                name = mod->customName;
            }

            PanPrint(
                "<%s '%s'>", PANTERM_MODULE,
                name != NULL ? name : PANTERM_UNKNOWN
            );
            break;
        }
        case OT_UPVAL: {
            PanPrint("<" PANTERM_UPVALUE ">");
            break;
        }

        // Needs Seen Guard
        case OT_ARR: {
            if (SeenSetEnter(seen, o)) {
                PanPrint("[...]");
                return;
            }
            const struct OArray *arr = &o->v.OArray;
            PanPrint("[");
            if (arr->items != NULL) {
                for (u64 i = 0; i < arrlen(arr->items); i++) {
                    PValue val = arr->items[i];
                    internalPrintValue(val, seen);
                    if (i != arr->count - 1) {
                        PanPrint(", ");
                    }
                }
            }
            PanPrint("]");
            SeenSetExit(seen, o);
            break;
        }

        case OT_MAP: {
            if (SeenSetEnter(seen, o)) {
                PanPrint("{...}");
                return;
            }
            const struct OMap *map = &o->v.OMap;
            PanPrint("{");
            if (map->table != NULL) {
                for (int i = 0; i < map->count; i++) {
                    PValue k = map->table[i].vkey;
                    PValue v = map->table[i].value;
                    internalPrintValue(k, seen);
                    PanPrint(" : ");
                    internalPrintValue(v, seen);
                    if (i + 1 != map->count) {
                        PanPrint(", ");
                    }
                }
            }

            PanPrint("}");
            SeenSetExit(seen, o);
            break;
        }
    }
}
