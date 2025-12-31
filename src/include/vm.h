#ifndef VM_H
#define VM_H

#include "object.h"
#include "opcode.h"
#include "ptypes.h"
#include "symtable.h"
#ifdef __cplusplus
extern "C" {
#endif

#ifndef PVM_STACK_SIZE
#define PVM_STACK_SIZE 2048
#endif

typedef struct Pgc Pgc;

typedef struct PVm {
    PValue *constPool;
    u16 constCount;
    PValue stack[PVM_STACK_SIZE];
    PValue *sp;

    u8 *code;
    u64 codeCount;
    u8 *ip;

    Pgc *gc;
    SymbolTable *globals;
} PVm;

PVm *NewVm(void);
void SetupVm(PVm *vm, Pgc *gc, PBytecode *bt);
void FreeVm(PVm *vm);
void DebugVMStack(PVm *vm);

void VmRun(PVm *vm);
#ifdef __cplusplus
}
#endif

#endif
