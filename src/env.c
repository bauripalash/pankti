#include "include/env.h"
#include "external/stb/stb_ds.h"
#include "include/alloc.h"
#include "include/object.h"
#include "include/utils.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

PEnv *NewEnv(PEnv *enclosing) {
    PEnv *e = PCreate(PEnv);
    // error check;
    e->count = 0;
    e->table = NULL;
    e->enclosing = enclosing;
    return e;
}

EnvPair *NewPair(uint32_t key, PObj *value) {
    EnvPair *p = PCreate(EnvPair);
    p->key = key;
    p->value = value;
    return p;
}

void FreeEnv(PEnv *e) {
    if (e == NULL) {
        return;
    }

    if (e->table != NULL) {
        for (int i = 0; i < e->count; i++) {
            EnvPair *pair = arrpop(e->table);
            free(pair);
        }

        arrfree(e->table);
    }

    free(e);
}

void DebugEnv(PEnv *e) {
    if (e == NULL) {
        return;
    }
    if (e->table == NULL) {
        return;
    }
    for (int i = 0; i < arrlen(e->table); i++) {
        EnvPair *p = e->table[i];
        printf("\n%d < %d '", i, p->key);
        PrintObject(p->value);
        printf("' >\n");
    }

    if (e->enclosing != NULL) {
        DebugEnv(e->enclosing);
    }
}

void EnvPutValue(PEnv *e, uint32_t hash, PObj *value) {
    EnvPair *newPair = NewPair(hash, value);
    arrput(e->table, newPair);
    e->count++;
}

bool EnvSetValue(PEnv *e, uint32_t hash, PObj *value) {
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

    for (int i = 0; i < arrlen(e->table); i++) {
        EnvPair *p = e->table[i];
        if (p->key == hash) {
            p->value = value;
            return true;
        }
    }

    if (e->enclosing != NULL) {
        return EnvSetValue(e->enclosing, hash, value);
    }

    return false;
}

PObj *EnvGetValue(PEnv *e, uint32_t hash) {
    if (e == NULL) {
        return NULL;
    }

    for (int i = 0; i < e->count; i++) {
        if (e->table[i]->key == hash) {
            return e->table[i]->value;
        }
    }

    if (e->enclosing != NULL) {
        return EnvGetValue(e->enclosing, hash);
    }

    return NULL;
}
