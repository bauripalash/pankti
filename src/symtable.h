/*
 * Copyright (c) 2022 Palash Bauri
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PANKTI_SYMTABLE_H
#define PANKTI_SYMTABLE_H

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
#include "external/verstable/verstable.h"
// NOLINTEND(clang-analyzer-*)

typedef struct Pgc Pgc;

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
void DebugSymbolTable(SymbolTable *table);
void MarkSymbolTable(Pgc *gc, SymbolTable *table);

#ifdef __cplusplus
}
#endif

#endif
