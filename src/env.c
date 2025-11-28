#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "include/env.h"
#include "external/stb/stb_ds.h"
#include "include/alloc.h"
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
