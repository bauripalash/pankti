#include "include/strpool.h"
#include "include/alloc.h"
#include "include/object.h"
#include "include/ptypes.h"
#include "include/utils.h"
#include <stdbool.h>
#include <stdlib.h>

// NOLINTBEGIN(clang-analyzer-*)
static inline u64 spHashFn(PObj *key) { return key->v.OString.hash; }

static inline bool spCompareFn(PObj *a, PObj *b) {
    if (a->v.OString.hash != b->v.OString.hash) {
        return false;
    }

    return StrEqual(a->v.OString.value, b->v.OString.value);
}

#define NAME     SPoolSet
#define KEY_TY   PObj *
#define MAX_LOAD 0.75
#define HASH_FN  spHashFn
#define CMPR_FN  spCompareFn
#define IMPLEMENTATION_MODE
#include "external/verstable/verstable.h"
// NOLINTEND

PStringPool *NewStringPool(void) {
    PStringPool *pool = PCreate(PStringPool);
    if (pool == NULL) {
        return NULL;
    }

    SPoolSet_init(&pool->table);
    pool->count = 0;
    return pool;
}
void FreeStringPool(PStringPool *sp) {
    if (sp == NULL) {
        return;
    }

    SPoolSet_cleanup(&sp->table);
    sp->count = 0;
    PFree(sp);
}

PObj *StringPoolFind(PStringPool *sp, const char *val, u64 hash) {
    if (sp == NULL || val == NULL) {
        return NULL;
    }

    PObj temp;
    temp.type = OT_STR;
    temp.v.OString.value = (char *)val;
    temp.v.OString.hash = hash;
    temp.v.OString.name = NULL;

    SPoolSet_itr it = SPoolSet_get(&sp->table, &temp);
    if (!SPoolSet_is_end(it)) {
        return it.data->key;
    }

    return NULL;
}

// NOLINTBEGIN
static inline void stringPoolInsertValue(PStringPool *table, PObj *obj) {
    if (table == NULL) {
        return;
    }
    // NOLINENEXTLINE
    SPoolSet_insert(&table->table, obj);
    table->count = (u64)SPoolSet_size(&table->table);
}
// NOLINTEND
bool StringPoolInsert(PStringPool *sp, PObj *strObj) {
    if (sp == NULL || strObj == NULL) {
        return false;
    }

    stringPoolInsertValue(sp, strObj);
    return true;
}
bool StringPoolRemove(PStringPool *sp, PObj *strObj) {
    if (sp == NULL || strObj == NULL) {
        return false;
    }

    SPoolSet_erase(&sp->table, strObj);
    sp->count = (u64)SPoolSet_size(&sp->table);
    return true;
}
void StringPoolMark(PStringPool *sp) {
    if (sp == NULL) {
        return;
    }

    for (SPoolSet_itr itr = SPoolSet_first(&sp->table); !SPoolSet_is_end(itr);
         itr = SPoolSet_next(itr)) {
        itr.data->key->marked = true;
    }
}
