#include "include/vm.h"
#include "include/alloc.h"
#include "include/object.h"
#include "include/opcode.h"
#include "include/printer.h"
#include "include/ptypes.h"
#include <stdbool.h>
#include <stdlib.h>

PVm *NewVm(void) {
    PVm *vm = PCreate(PVm);
    vm->constPool = NULL;
    vm->constCount = 0;
    vm->sp = 0;
    vm->code = NULL;
    vm->constCount = 0;

    return vm;
}

void SetupVm(PVm *vm, PBytecode *bt) {
    vm->constPool = bt->constPool;
    vm->constCount = bt->constCount;
    vm->code = bt->code;
    vm->codeCount = bt->codeCount;
}
void FreeVm(PVm *vm) { PFree(vm); }

void DebugVMStack(PVm *vm) {
    PanPrint("==== STACK ====\n");
    for (int i = 0; i < vm->sp; i++) {
        PanPrint("[ %d | ", i);
        PrintValue(vm->stack[i]);
        PanPrint(" ]\n");
    }
    PanPrint("== END STACK ==\n");
}

static bool vmPush(PVm *vm, PValue val) {
    if (vm->sp >= PVM_STACK_SIZE) {
        return false;
    }

    vm->stack[vm->sp] = val;
    vm->sp++;
    return true;
}

static PValue vmPop(PVm *vm) {
    if (vm->sp <= 0) {
        return MakeNil();
    }

    PValue val = vm->stack[vm->sp - 1];
    vm->sp--;
    return val;
}

static u16 readU16(PVm *vm, u64 ip) { return ReadU16RawCode(vm->code, ip + 1); }

void VmRun(PVm *vm) {
    for (u64 ip = 0; ip < vm->codeCount; ip++) {
        PanOpCode op = vm->code[ip];
        DebugVMStack(vm);
        switch (op) {
            case OP_CONST: {
                u16 constIndex = readU16(vm, ip);
                vmPush(vm, vm->constPool[constIndex]);
                ip += 2;
                break;
            }
            case OP_ADD: {
                PValue right = vmPop(vm);
                PValue left = vmPop(vm);
                if (IsValueNum(left) && IsValueNum(right)) {
                    double res = ValueAsNum(left) + ValueAsNum(right);
                    vmPush(vm, MakeNumber(res));
                }
                break;
            }

            default: break;
        }
    }
}
