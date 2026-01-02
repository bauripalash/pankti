#ifndef VM_H
#define VM_H

#include "object.h"
#include "opcode.h"
#include "ptypes.h"
#include "symtable.h"
#ifdef __cplusplus
extern "C" {
#endif

// VM Stack size (can be modified at compile time)
#ifndef PVM_STACK_SIZE
#define PVM_STACK_SIZE 2048
#endif

// Forward declaration of GC
typedef struct Pgc Pgc;

// Pankti Virtual Machine Object
typedef struct PVm {
    // Constants array
    PValue *constPool;
    // Constants array count
    u16 constCount;

    // The Stack
    PValue stack[PVM_STACK_SIZE];
    // Stack pointer
    PValue *sp;

    // Raw bytecode codes
    u8 *code;
    // count
    u64 codeCount;
    // Instruction Pointer
    u8 *ip;

    // Garbage Collector
    Pgc *gc;
    // Globals hash table
    SymbolTable *globals;
} PVm;

// Create an empty VM Object
PVm *NewVm(void);
// Setup VM with bytecode, constants, gc etc.
void SetupVm(PVm *vm, Pgc *gc, PBytecode *bt);
// Free the VM
void FreeVm(PVm *vm);
// Debug VM Stack
void DebugVMStack(PVm *vm);
// Run the VM
void VmRun(PVm *vm);
#ifdef __cplusplus
}
#endif

#endif
