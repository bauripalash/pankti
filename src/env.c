#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "external/stb/stb_ds.h"
#include "include/alloc.h"
#include "include/env.h"
#include "include/gc.h"
#include "include/object.h"
#include "include/ptypes.h"

PEnv *NewEnv(PEnv *enclosing) {
    PEnv *e = PCreate(PEnv);
    // error check;
    e->count = 0;
    e->table = NULL;
    e->enclosing = enclosing;
    return e;
}

void FreeEnv(PEnv *e) {
    if (e == NULL) {
        return;
    }

    if (e->table != NULL) {
        hmfree(e->table);
    }
    PFree(e);
}

u64 EnvGetCount(const PEnv *e) {
    if (e == NULL || e->table == NULL) {
        return UINT64_MAX;
    }

    return (u64)hmlen(e->table);
}

void EnvTableAddValue(PEnv *e, u64 hash, PValue value) {
    if (e == NULL)
        return;
    hmput(e->table, hash, value);
}

void MarkEnvGC(Pgc *gc, PEnv *e) {
    if (gc == NULL || e == NULL) {
        return;
    }

    if (e->table == NULL) {
        return;
    }

    u64 envCount = EnvGetCount(e);

    for (u64 i = 0; i < envCount; i++) {
        GcMarkValue(gc, e->table[i].value);
    }
}

void EnvCaptureUpvalues(Pgc *gc, PEnv *parentEnv, PEnv *clsEnv) {
    if (parentEnv == NULL || clsEnv == NULL) {
        return;
    }

    PEnv *cur = parentEnv;
    while (cur != NULL) {
        if (cur->table != NULL) {
            u64 curCount = cur->count;
            for (u64 i = 0; i < curCount; i++) {
                u64 key = cur->table[i].key;
                if (EnvHasKey(clsEnv, key)) {
                    continue;
                }

                PValue curVal = cur->table[i].value;
                PValue upVal;
                PObj *maybe = IsValueObj(curVal) ? ValueAsObj(curVal) : NULL;

                if (maybe != NULL && maybe->type == OT_UPVAL) {
                    upVal = curVal;
                } else {
                    PObj *upObj = NewUpvalueObject(gc, curVal);
                    upVal = MakeObject(upObj);
                    // hmput(cur->table, key, upVal); // Upgrade parent env's
                    // upval
                    EnvTableAddValue(cur, key, upVal);
                    cur->count = EnvGetCount(cur);
                }

                // hmput(clsEnv->table, key, upVal);
                EnvTableAddValue(clsEnv, key, upVal);
            }
        }
        cur = cur->enclosing;
    }

    // clsEnv->count = (u64)hmlen(clsEnv->table);
    clsEnv->count = EnvGetCount(clsEnv);
}

void DebugEnv(PEnv *e) {
    if (e == NULL) {
        return;
    }
    if (e->table == NULL) {
        return;
    }
    u64 count = (u64)hmlen(e->table);
    for (u64 i = 0; i < count; i++) {
        printf("\n%zu < %ld '", i, e->table[i].key);
        PrintValue(e->table[i].value);
        printf("' >\n");
    }

    if (e->enclosing != NULL) {
        DebugEnv(e->enclosing);
    }
}

bool EnvHasKey(PEnv *e, u64 hash) {
    if (e == NULL || e->table == NULL) {
        return false;
    }
    if (hmgeti(e->table, hash) >= 0) {
        return true;
    }

    return false;
}

void EnvPutValue(PEnv *e, u64 hash, PValue value) {
    if (e == NULL) {
        return;
    }

    if (e->table != NULL) {
        long idx = hmgeti(e->table, hash);
        if (idx > -1) {
            PValue stored = hmget(e->table, hash);
            if (IsValueObjType(stored, OT_UPVAL)) {
                // If already existing value is upvalue just update its cell
                ValueAsObj(stored)->v.OUpval.value = value;
                return;
            }
        }
    }

    hmput(e->table, hash, value);
    e->count = (u64)hmlen(e->table);
}

bool EnvSetValue(PEnv *e, u64 hash, PValue value) {
    if (e == NULL) {
        return false;
    }
    if (e->table == NULL) {
        if (e->enclosing != NULL) {
            return EnvSetValue(e->enclosing, hash, value);
        } else {
            return false;
        }
    }

    if (hmgeti(e->table, hash) > -1) {
        PValue stored = hmget(e->table, hash);
        if (IsValueObjType(stored, OT_UPVAL)) {
            // If the prexisting value is a upvalue, just update its cell value
            ValueAsObj(stored)->v.OUpval.value = value;
            return true;
        }
        hmput(e->table, hash, value);
        return true;
    }

    if (e->enclosing != NULL) {
        return EnvSetValue(e->enclosing, hash, value);
    }

    return false;
}

PValue EnvGetValue(PEnv *e, u64 hash, bool *found) {
    if (e == NULL) {
        *found = false;
        return MakeNil();
    }

    if (hmgeti(e->table, hash) > -1) {
        *found = true;
        PValue stored = hmget(e->table, hash);
        if (IsValueObjType(stored, OT_UPVAL)) {
            return ValueAsObj(stored)->v.OUpval.value;
        }
        return stored;
    }

    if (e->enclosing != NULL) {
        return EnvGetValue(e->enclosing, hash, found);
    }

    *found = false;
    return MakeNil();
}
