#include "include/vm.h"
#include "include/alloc.h"
#include "include/compiler.h"
#include "include/gc.h"
#include "include/object.h"
#include "include/opcode.h"
#include "include/printer.h"
#include "include/ptypes.h"
#include "include/utils.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

PVm *NewVm(void) {
    PVm *vm = PCreate(PVm);
    vm->constPool = NULL;
    vm->constCount = 0;
    vm->sp = vm->stack;
    vm->code = NULL;
    vm->constCount = 0;
    vm->ip = 0;
    vm->gc = NULL;

    return vm;
}

void SetupVm(PVm *vm, Pgc *gc, PBytecode *bt) {
    vm->constPool = bt->constPool;
    vm->constCount = bt->constCount;
    vm->code = bt->code;
    vm->ip = vm->code;
    vm->codeCount = bt->codeCount;
    vm->gc = gc;
}
void FreeVm(PVm *vm) { PFree(vm); }

void DebugVMStack(PVm *vm) {
    PanPrint("==== STACK ====\n");
    int i = 0;
    for (PValue *val = vm->stack; val < vm->sp; val++) {
        PanPrint("[ |%d| ", i);
        PrintValue(*val);
        PanPrint(" ]\n");
        i++;
    }
    PanPrint("== END STACK ==\n");
}

PValue VmGetLastPopped(const PVm *vm) { return MakeNil(); }

static bool vmPush(PVm *vm, PValue val) {
    *vm->sp = val;
    vm->sp++;
    return true;
}

static PValue vmPop(PVm *vm) {
    vm->sp--;
    return *vm->sp;
}

static PValue vmPeek(const PVm *vm, int index) {

    PValue val = vm->sp[-1 - index];
    return val;
}

static bool vmBinaryOpNumber(PVm *vm, PanOpCode op, PValue left, PValue right) {
    double leftVal = ValueAsNum(left);
    double rightVal = ValueAsNum(right);
    double result = 0.0;
    switch (op) {
        case OP_ADD: result = leftVal + rightVal; break;
        case OP_SUB: result = leftVal - rightVal; break;
        case OP_MUL: result = leftVal * rightVal; break;
        case OP_DIV: result = leftVal / rightVal; break;
        case OP_EXPONENT: result = pow(leftVal, rightVal); break;
        default: return false;
    }
    vmPop(vm);
    vmPop(vm);
    return vmPush(vm, MakeNumber(result));
}

static bool vmBinaryOpString(PVm *vm, PanOpCode op, PValue left, PValue right) {
    struct OString *ls = &ValueAsObj(left)->v.OString;
    u64 lsLen = (u64)strlen(ls->value);
    struct OString *rs = &ValueAsObj(right)->v.OString;
    u64 rsLen = (u64)strlen(rs->value);
    bool ok = true;
    char *newStr = StrJoin(ls->value, lsLen, rs->value, rsLen, &ok);
    if (!ok) {
        return MakeNil();
    }

    PObj *nsObj = NewStrObject(vm->gc, NULL, newStr, true); // fetch the token
    vmPop(vm);
    vmPop(vm);

    return vmPush(vm, MakeObject(nsObj));
}

static bool vmBinaryOp(PVm *vm, PanOpCode op) {
    PValue right = vmPeek(vm, 0);
    PValue left = vmPeek(vm, 1);

    if (IsValueNum(left) && IsValueNum(right)) {
        bool isok = vmBinaryOpNumber(vm, op, left, right);
        if (!isok) {
            PanPrint("Failed to binary operation\n");
            return false;
        }
    } else if (IsValueObjType(left, OT_STR) && IsValueObjType(right, OT_STR)) {
        bool isok = vmBinaryOpString(vm, op, left, right);
        if (!isok) {
            PanPrint("Failed to binary operation on string\n");
            return false; // todo handle better
        }
    }

    return false;
}

static bool vmCompareOp(PVm *vm, PanOpCode op) {
    PValue right = vmPeek(vm, 0);
    PValue left = vmPeek(vm, 1);

    if (IsValueNum(left) && IsValueNum(right)) {
        bool result = false;
        switch (op) {
            case OP_GT: result = left > right; break;
            case OP_GTE: result = left >= right; break;
            case OP_LT: result = left < right; break;
            case OP_LTE: result = left <= right; break;
            default: break; // should never reach here
        }

        vmPop(vm);
        vmPop(vm);
        vmPush(vm, MakeBool(result));
        return true;
    } else {
        PanPrint("Invalid Compare Operation!\n");
        return false;
    }
}

static finline u8 readByte(PVm *vm) { return *vm->ip++; }

static finline u16 readShort(PVm *vm) {
    vm->ip += 2;
    return ((u16)(vm->ip[-2] << 8) | (u16)(vm->ip[-1]));
}

static finline PValue readConst(PVm *vm) {
    return vm->constPool[readShort(vm)];
}

void VmRun(PVm *vm) {
    while (true) {
        u8 ins;

        switch (ins = readByte(vm)) {
            case OP_DEBUG: {
                PrintValue(vmPop(vm));
                PanPrint("\n");
                return;
            }

            case OP_CONST: {
                PValue val = readConst(vm);
                vmPush(vm, val);
                break;
            }
            case OP_TRUE: {
                vmPush(vm, MakeBool(true));
                break;
            }
            case OP_FALSE: {
                vmPush(vm, MakeBool(false));
                break;
            }
            case OP_NIL: {
                vmPush(vm, MakeNil());
                break;
            }

            case OP_ADD:
            case OP_SUB:
            case OP_MUL:
            case OP_DIV:
            case OP_EXPONENT: {
                vmBinaryOp(vm, ins);
                break;
            }

            case OP_EQUAL:
            case OP_NOTEQUAL: {
                PValue b = vmPeek(vm, 0);
                PValue a = vmPeek(vm, 1);
                bool result = IsValueEqual(a, b);
                if (ins == OP_NOTEQUAL) {
                    result = !result;
                }
                vmPop(vm);
                vmPop(vm);
                vmPush(vm, MakeBool(result));
                break;
            }
            case OP_GT:
            case OP_GTE:
            case OP_LT:
            case OP_LTE: {
                vmCompareOp(vm, ins);
                break;
            }

            case OP_NEGATE: {
                if (!IsValueNum(vmPeek(vm, 0))) {
                    break;
                }

                vmPush(vm, MakeNumber(-ValueAsNum(vmPop(vm))));
                break;
            }
            case OP_NOT: {
                vmPush(vm, MakeBool(!IsValueTruthy(vmPop(vm))));
            }
        }

        // PanPrint("== %s ==\n", GetOpDefinition(ins).name);
        // DebugVMStack(vm);
    }
}
