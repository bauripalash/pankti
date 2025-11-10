#include "include/env.h"
#include "external/stb/stb_ds.h"
#include "include/alloc.h"
#include "include/object.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

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
    for (int i = 0; i < hmlen(e->table); i++) {
        printf("\n%d < %d '", i, e->table[i].key);
        PrintValue(&e->table[i].value);
        printf("' >\n");
    }

    if (e->enclosing != NULL) {
        DebugEnv(e->enclosing);
    }
}

void EnvPutValue(PEnv *e, uint32_t hash, PValue value) {
    if (e == NULL) {
        return;
    }
    hmput(e->table, hash, value);
    e->count = hmlen(e->table);
}

bool EnvSetValue(PEnv *e, uint32_t hash, PValue value) {
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
        hmput(e->table, hash, value);
        return true;
    }

    if (e->enclosing != NULL) {
        return EnvSetValue(e->enclosing, hash, value);
    }

    return false;
}

PValue EnvGetValue(PEnv *e, uint32_t hash, bool *found) {
    if (e == NULL) {
        *found = false;
        return MakeNil();
    }

    if (hmgeti(e->table, hash) > -1) {
        *found = true;
        return hmget(e->table, hash);
    }

    if (e->enclosing != NULL) {
        return EnvGetValue(e->enclosing, hash, found);
    }

    *found = false;
    return MakeNil();
}
