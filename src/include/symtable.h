#ifndef SYMTABLE_H
#define SYMTABLE_H

#ifdef __cplusplus
extern "C" {
#endif
#include "object.h"
#include "ptypes.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// NOLINTBEGIN(clang-analyzer-*)
#define NAME   SymTable
#define KEY_TY PObj *
#define VAL_TY PValue
#define HEADER_MODE
#include "../external/verstable/verstable.h"
// NOLINTEND(clang-analyzer-*)

typedef struct SymbolTable {
    SymTable table;
    u64 count;
} SymbolTable;

SymbolTable *NewSymbolTable(void);
void FreeSymbolTable(SymbolTable *table);
u64 SymbolTableGetCount(const SymbolTable *table);
bool SymbolTableHasKey(const SymbolTable *table, PObj *str);
PValue SymbolTableFind(SymbolTable *table, PObj *str, bool *found);
bool SymbolTableSet(SymbolTable *table, PObj *str, PValue value);
bool SymbolTableRemove(SymbolTable *table, PObj *str);
bool SymbolTableAddAll(SymbolTable *from, SymbolTable *to);

#ifdef __cplusplus
}
#endif

#endif
