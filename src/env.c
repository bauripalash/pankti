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

// NOLINTBEGIN
#define NAME   EnvTable
#define KEY_TY u64
#define VAL_TY PValue
#define IMPLEMENTATION_MODE
#include "external/verstable/verstable.h"
// NOLINTEND

PEnv *NewEnv(Pgc *gc, PEnv *enclosing) {
    if (gc->envFreeListCount > 0) {
        PEnv *e = arrpop(gc->envFreeList);
        e->enclosing = enclosing;
        gc->envFreeListCount = (u64)arrlen(gc->envFreeList);
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
    EnvTable_clear(&e->table);
    arrpush(gc->envFreeList, e);
    gc->envFreeListCount = (u64)arrlen(gc->envFreeList);
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

//NOLINTBEGIN
static inline void envTableInsertValue(PEnv *e, u64 hash, PValue value) {
    if (e == NULL) {
        return;
    }
	//NOLINENEXTLINE
    EnvTable_insert(&e->table, hash, value);
    e->count = EnvGetCount(e);
}
//NOLINTEND

void MarkEnvGC(Pgc *gc, PEnv *e) {
    if (gc == NULL || e == NULL) {
        return;
    }

    for (EnvTable_itr itr = vt_first(&e->table); !vt_is_end(itr);
         itr = vt_next(itr)) {
        GcMarkValue(gc, itr.data->val);
    }
}

void EnvCaptureUpvalues(Pgc *gc, PEnv *parentEnv, PEnv *clsEnv) {
    if (parentEnv == NULL || clsEnv == NULL) {
        return;
    }

    PEnv *cur = parentEnv;
    while (cur != NULL) {
        for (EnvTable_itr itr = vt_first(&cur->table); !vt_is_end(itr);
             itr = vt_next(itr)) {
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
    for (EnvTable_itr itr = vt_first(&e->table); !vt_is_end(itr);
         itr = vt_next(itr)) {
        printf("%ld| <%ld '", i, itr.data->key);
        PrintValue(itr.data->val);
        printf("'>\n");
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

    if (!vt_is_end(it)) {
        return true;
    }

    return false;
}

// Dont lint the next function. all the warnings are from external library
//NOLINTBEGIN
void EnvPutValue(PEnv *e, u64 hash, PValue value) {

    if (e == NULL) {
        return;
    }
    EnvTable_itr it = EnvTable_get(&e->table, hash);
    if (!vt_is_end(it)) { 
		PValue stored = it.data->val;
        if (IsValueObjType(stored, OT_UPVAL)) {
            ValueAsObj(stored)->v.OUpval.value = value;
            return;
        }else{
			it.data->val = value;
			return;
		}
    } else{
		envTableInsertValue(e, hash, value); 
	}
}
//NOLINTEND

bool EnvSetValue(PEnv *e, u64 hash, PValue value) {
    if (e == NULL) {
        return false;
    }

    EnvTable_itr it = EnvTable_get(&e->table, hash);
    if (!vt_is_end(it)) {
        PValue stored = it.data->val;
        if (IsValueObjType(stored, OT_UPVAL)) {
            ValueAsObj(stored)->v.OUpval.value = value;
            return true;
        }else{
			//envTableInsertValue(e, hash, value);
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
    if (!vt_is_end(it)) {
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
