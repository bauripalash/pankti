#include "include/symtable.h"
#include "include/alloc.h"
#include "include/object.h"
#include "include/ptypes.h"
#include <stdbool.h>
#include <stdlib.h>

// NOLINTBEGIN
static inline uint64_t symHashFn(PObj *key) { return key->v.OString.hash; }
static inline bool symCompareFn(PObj *key1, PObj *key2) {
    return key1->v.OString.hash == key2->v.OString.hash;
}
#define NAME     SymTable
#define KEY_TY   PObj *
#define VAL_TY   PValue
#define MAX_LOAD 0.75
#define HASH_FN  symHashFn
#define CMPR_FN  symCompareFn
#define IMPLEMENTATION_MODE
#include "external/verstable/verstable.h"
// NOLINTEND

SymbolTable *NewSymbolTable(void) {
    SymbolTable *table = PCreate(SymbolTable);
    table->count = 0;
    SymTable_init(&table->table);
    return table;
}
void FreeSymbolTable(SymbolTable *table) {
    if (table == NULL) {
        return;
    }
    SymTable_cleanup(&table->table);
    table->count = 0;
    PFree(table);
}

u64 SymbolTableGetCount(const SymbolTable *table) {
    if (table == NULL) {
        return 0;
    }

    return (u64)SymTable_size(&table->table);
}

// NOLINTBEGIN
static inline void symbolTableInsertValue(
    SymbolTable *table, PObj *obj, PValue value
) {
    if (table == NULL) {
        return;
    }
    // NOLINENEXTLINE
    SymTable_insert(&table->table, obj, value);
    table->count = SymbolTableGetCount(table);
}
// NOLINTEND

bool SymbolTableHasKey(const SymbolTable *table, PObj *str) {
    if (table == NULL || str == NULL) {
        return false;
    }

    if (str->type != OT_STR) {
        return false;
    }
    SymTable_itr it = SymTable_get(&table->table, str);
    if (!SymTable_is_end(it)) {
        return true;
    }

    return false;
}

PValue SymbolTableFind(SymbolTable *table, PObj *str, bool *found) {
    if (table == NULL || str == NULL) {
        *found = false;
        return MakeNil();
    }

    if (str->type != OT_STR) {
        *found = false;
        return MakeNil();
    }

    SymTable_itr it = SymTable_get(&table->table, str);
    if (!SymTable_is_end(it)) {
        *found = true;
        PValue stored = it.data->val;
        return stored;
    }

    *found = false;
    return MakeNil();
}

// NOLINTBEGIN
bool SymbolTableSet(SymbolTable *table, PObj *str, PValue value) {
    if (table == NULL || str == NULL) {
        return false;
    }

    if (str->type != OT_STR) {
        return false;
    }

    SymTable_itr it = SymTable_get(&table->table, str);
    if (!SymTable_is_end(it)) {
        it.data->val = value;
        return true;
    } else {
        symbolTableInsertValue(table, str, value);
        return true;
    }
}

// NOLINTEND
bool SymbolTableRemove(SymbolTable *table, PObj *str) {
    if (table == NULL || str == NULL) {
        return false;
    }

    if (str->type != OT_STR) {
        return false;
    }

    SymTable_erase(&table->table, str);
    return true;
}
bool SymbolTableAddAll(SymbolTable *from, SymbolTable *to) {
    if (from == NULL || to == NULL) {
        return false;
    }

    for (SymTable_itr itr = SymTable_first(&from->table); !SymTable_is_end(itr);
         itr = SymTable_next(itr)) {
        SymbolTableSet(to, itr.data->key, itr.data->val);
    }

    return true;
}
