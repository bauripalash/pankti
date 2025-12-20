#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "include/alloc.h"
#include "include/env.h"
#include "include/gc.h"
#include "include/object.h"
#include "include/ptypes.h"
#include "include/printer.h"

// NOLINTBEGIN
static inline uint64_t envHashFn(u64 key) { return key; }
static inline bool envCompareFn(u64 key1, u64 key2) { return key1 == key2; }
#define NAME     EnvTable
#define KEY_TY   u64
#define VAL_TY   PValue
#define MAX_LOAD 0.75
#define HASH_FN  envHashFn
#define CMPR_FN  envCompareFn
#define IMPLEMENTATION_MODE
#include "external/verstable/verstable.h"
// NOLINTEND

PEnv *NewEnv(Pgc *gc, PEnv *enclosing) {
    if (gc->envFreeListCount > 0) {
        gc->envFreeListCount--;
        PEnv *e = gc->envFreeList[gc->envFreeListCount];
        e->enclosing = enclosing;
        return e;
    }

    PEnv *e = PCreate(PEnv);
    // error check;
    e->count = 0;
    // e->table = NULL;
    EnvTable_init(&e->table);
    // EnvTable_reserve(&e->table, 8);
    e->enclosing = enclosing;
    return e;
}

void RecycleEnv(Pgc *gc, PEnv *e) {
    e->enclosing = NULL;
    e->count = 0;

    if (gc->envFreeListCount >= gc->envFreeListCap) {
        u64 newCap = gc->envFreeListCap * GC_ENV_FREELIST_GROW_FACTOR;
        PEnv **temp = PRealloc(gc->envFreeList, sizeof(PEnv *) * newCap);
        if (temp == NULL) {
            ReallyFreeEnv(e);
            return;
        }
        gc->envFreeList = temp;
        gc->envFreeListCap = newCap;
    }

    EnvTable_clear(&e->table);
    gc->envFreeList[gc->envFreeListCount] = e;
    gc->envFreeListCount++;
}

void ReallyFreeEnv(PEnv *e) {
    if (e == NULL) {
        return;
    }

    EnvTable_cleanup(&e->table);
    PFree(e);
}

u64 EnvGetCount(const PEnv *e) {
    if (e == NULL) {
        return UINT64_MAX;
    }

    return (u64)EnvTable_size(&e->table);
}

// NOLINTBEGIN
static inline void envTableInsertValue(PEnv *e, u64 hash, PValue value) {
    if (e == NULL) {
        return;
    }
    // NOLINENEXTLINE
    EnvTable_insert(&e->table, hash, value);
    e->count = EnvGetCount(e);
}
// NOLINTEND

void MarkEnvGC(Pgc *gc, PEnv *e) {
    if (gc == NULL || e == NULL) {
        return;
    }

    for (EnvTable_itr itr = EnvTable_first(&e->table); !EnvTable_is_end(itr);
         itr = EnvTable_next(itr)) {
        GcMarkValue(gc, itr.data->val);
    }
}

void EnvCaptureUpvalues(Pgc *gc, PEnv *parentEnv, PEnv *clsEnv) {
    if (parentEnv == NULL || clsEnv == NULL) {
        return;
    }

    PEnv *cur = parentEnv;
    while (cur != NULL) {
        for (EnvTable_itr itr = EnvTable_first(&cur->table);
             !EnvTable_is_end(itr); itr = EnvTable_next(itr)) {
            u64 key = itr.data->key;
            if (EnvHasKey(clsEnv, key)) {
                continue;
            }

            PValue curVal = itr.data->val;
            PValue upVal;

            if (IsValueObjType(curVal, OT_UPVAL)) {
                upVal = curVal;
            } else {
                // Upgrade cur env's value to upvalue;
                PObj *curUpObj = NewUpvalueObject(gc, curVal);
                upVal = MakeObject(curUpObj); // <- upgraded upval's value
                envTableInsertValue(cur, key, upVal);
            }

            envTableInsertValue(clsEnv, key, upVal);
        }
        cur = cur->enclosing;
    }

    clsEnv->count = EnvGetCount(clsEnv);
}

void DebugEnv(PEnv *e) {
    if (e == NULL) {
        return;
    }

    u64 i = 0;
    for (EnvTable_itr itr = EnvTable_first(&e->table); !EnvTable_is_end(itr);
         itr = EnvTable_next(itr)) {
        PanPrint("%ld| <%ld '", i, itr.data->key);
        PrintValue(itr.data->val);
        PanPrint("'>\n");
        i++;
    }

    if (e->enclosing != NULL) {
        DebugEnv(e->enclosing);
    }
}

bool EnvHasKey(PEnv *e, u64 hash) {
    if (e == NULL) {
        return false;
    }

    EnvTable_itr it = EnvTable_get(&e->table, hash);

    if (!EnvTable_is_end(it)) {
        return true;
    }

    return false;
}

// Dont lint the next function. all the warnings are from external library
// NOLINTBEGIN
void EnvPutValue(PEnv *e, u64 hash, PValue value) {

    if (e == NULL) {
        return;
    }
    EnvTable_itr it = EnvTable_get(&e->table, hash);
    if (!EnvTable_is_end(it)) {
        PValue stored = it.data->val;
        if (IsValueObjType(stored, OT_UPVAL)) {
            ValueAsObj(stored)->v.OUpval.value = value;
            return;
        } else {
            it.data->val = value;
            return;
        }
    } else {
        envTableInsertValue(e, hash, value);
    }
}
// NOLINTEND

bool EnvSetValue(PEnv *e, u64 hash, PValue value) {
    if (e == NULL) {
        return false;
    }

    EnvTable_itr it = EnvTable_get(&e->table, hash);
    if (!EnvTable_is_end(it)) {
        PValue stored = it.data->val;
        if (IsValueObjType(stored, OT_UPVAL)) {
            ValueAsObj(stored)->v.OUpval.value = value;
            return true;
        } else {
            // envTableInsertValue(e, hash, value);
            it.data->val = value;
            return true;
        }
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

    EnvTable_itr it = EnvTable_get(&e->table, hash);
    if (!EnvTable_is_end(it)) {
        *found = true;
        PValue stored = it.data->val;
        if (IsValueObjType(stored, OT_UPVAL)) {
            return ValueAsObj(stored)->v.OUpval.value;
        } else {
            return stored;
        }
    }

    if (e->enclosing != NULL) {
        return EnvGetValue(e->enclosing, hash, found);
    }

    *found = false;
    return MakeNil();
}
