/*
 * Copyright (c) 2022 Palash Bauri
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "pstdlib.h"
#include "gc.h"
#include "vm.h"
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

void PushStdlib(PVm *vm, SymbolTable *table, const char *name, StdlibMod mod) {
    if (vm == NULL || vm->gc == NULL) {
        return;
    }

    if (table == NULL) {
        return;
    }
    switch (mod) {
        case STDLIB_SYSTEM: PushStdlibSystem(vm, table); break;
        case STDLIB_MATH: PushStdlibMath(vm, table); break;
        case STDLIB_MAP: PushStdlibMap(vm, table); break;
        case STDLIB_STRING: PushStdlibString(vm, table); break;
        case STDLIB_ARRAY: PushStdlibArray(vm, table); break;
        case STDLIB_GRAPHICS: PushStdlibGraphics(vm, table); break;
        case STDLIB_FILE: PushStdlibFile(vm, table); break;
        case STDLIB_NONE: break;
    }
}

void PushStdlibEntries(
    PVm *vm,
    SymbolTable *table,
    const char *module,
    StdlibEntry *entries,
    u64 count
) {
    for (u64 i = 0; i < count; i++) {
        const StdlibEntry *entry = &entries[i];
        PObj *stdNameObj = NewStrObject(vm->gc, NULL, entry->name, false);
        VmPush(vm, MakeObject(stdNameObj));
        const char *entryName = StrFormat(
            "<%s>.%s", module != NULL ? module : "unknown", entry->name
        );
        PObj *stdFnObj =
            NewNativeFnObject(vm->gc, entryName, entry->fn, entry->arity);
        VmPush(vm, MakeObject(stdFnObj));
        SymbolTableSet(table, stdNameObj, VmPop(vm));
        VmPop(vm);
    }
}
