#ifndef VM_H
#define VM_H

#include "object.h"
#include "opcode.h"
#include "ptypes.h"
#include "symtable.h"
#ifdef __cplusplus
extern "C" {
#endif

// VM Per Frame Stack size (can be modified at compile time)
#ifndef PVM_PERFRAME_STACK_SIZE
#define PVM_PERFRAME_STACK_SIZE 256
#endif

// VM Call stack size
#ifndef PVM_FRAMESTACK_SIZE
#define PVM_FRAMESTACK_SIZE 64
#endif

#ifndef PVM_STACK_SIZE
#define PVM_STACK_SIZE PVM_PERFRAME_STACK_SIZE *PVM_FRAMESTACK_SIZE
#endif

// Forward declaration of GC
typedef struct Pgc Pgc;
typedef struct PanktiCore PanktiCore;

typedef struct PCallFrame {
    PObj *f;
    u8 *ip;
    PValue *slots;
} PCallFrame;

// Pankti Virtual Machine Object
typedef struct PVm {
    // Constants array
    // PValue *constPool;
    // Constants array count
    // u16 constCount;

    PCallFrame frames[PVM_FRAMESTACK_SIZE];
    int frameCount;

    // The Stack
    PValue stack[PVM_STACK_SIZE];
    // Stack pointer
    PValue *sp;

    // Raw bytecode codes
    // u8 *code;
    // count
    // u64 codeCount;
    // Instruction Pointer
    // u8 *ip;

    // Garbage Collector
    Pgc *gc;
    // Globals hash table
    SymbolTable *globals;

    // Link to pankti core
    PanktiCore *core;
} PVm;

// Create an empty VM Object
PVm *NewVm(PanktiCore *core);
// Setup VM with bytecode, constants, gc etc.
void SetupVm(PVm *vm, Pgc *gc, PObj *func);
// Free the VM
void FreeVm(PVm *vm);
// Debug VM Stack
void DebugVMStack(PVm *vm);
// Run the VM
void VmRun(PVm *vm);

// Push Value to VM Stack
bool VmPush(PVm *vm, PValue val);
// Pop Value from VM Stack
PValue VmPop(PVm *vm);
// Peek at VM Stack
PValue VmPeek(const PVm *vm, int index);

#ifdef __cplusplus
}
#endif

#endif
