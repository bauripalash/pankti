#ifndef VM_H
#define VM_H

#include "errctx.h"
#include "object.h"
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

typedef enum PModType {
    PMOD_STDLIB,
    PMOD_SCRIPT,
} PModType;

typedef struct PModule {
    PModType type;
    SymbolTable *table;
    char *pathname;
} PModule;

typedef struct ModProxyEntry {
    u64 key;
    char *name;
    PModule *mod;
} ModProxyEntry;

typedef struct PCallFrame {
    PObj *cls;
    u8 *ip;
    PValue *slots;
} PCallFrame;

// Pankti Virtual Machine Object
typedef struct PVm {
    PCallFrame frames[PVM_FRAMESTACK_SIZE];
    int frameCount;

    // The Stack
    PValue stack[PVM_STACK_SIZE];
    // Stack pointer
    PValue *sp;

    PModule **modules;
    u64 modCount;

    ModProxyEntry *modProxies;
    u64 modProxiesCount;

    // Garbage Collector
    Pgc *gc;
    // Globals hash table
    SymbolTable *globals;
    // Open Upvalues (Upvalues which are still in stack)
    PObj *openUpvals;

    // Error context
    PErrorCtx errCtx;

    // Path to the script path which is running
    const char *scriptPath;
    // Arguments to scripts
    char **scriptArgs;
    // Script argument count
    int scriptArgCount;
} PVm;

// Create an empty VM Object
PVm *NewVm(Pgc *gc, PErrorCtx errCtx);
// Setup VM with bytecode, constants, gc etc.
void SetupVm(
    PVm *vm, PObj *func, const char *scriptPath, int sArgc, char **sArgs
);
// Free the VM
void FreeVm(PVm *vm);

void VmMarkRoots(Pgc *gc, void *ctx);

// Debug VM Stack
void DebugVMStack(PVm *vm);
// Print Call Stack Trace
void VmPrintStackTrace(const PVm *vm);
// Run the VM
void VmRun(PVm *vm);

// Throw VM Runtime Error
void VmError(PVm *vm, const char *msg);

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
