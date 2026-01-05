#include "include/vm.h"
#include "include/alloc.h"
#include "include/compiler.h"
#include "include/core.h"
#include "include/gc.h"
#include "include/object.h"
#include "include/opcode.h"
#include "include/printer.h"
#include "include/ptypes.h"
#include "include/symtable.h"
#include "include/utils.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

static bool vmPush(PVm *vm, PValue val);
static PValue vmPop(PVm *vm);

PVm *NewVm(PanktiCore *core) {
    PVm *vm = PCreate(PVm);
    vm->sp = vm->stack;
    vm->gc = core->gc;
    vm->frameCount = 0;
    vm->globals = NewSymbolTable();

    return vm;
}

void SetupVm(PVm *vm, Pgc *gc, PObj *func) {
    vm->gc = gc;
    vmPush(vm, MakeObject(func));
    PCallFrame *frame = &vm->frames[vm->frameCount++];
    frame->f = func;
    frame->ip = func->v.OComFunction.code->code;
    frame->slots = vm->stack;
}
void FreeVm(PVm *vm) {
    if (vm == NULL) {
        return;
    }

    FreeSymbolTable(vm->globals);

    PFree(vm);
}

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

static void vmPrintStackTrace(PVm *vm) {
    for (int i = vm->frameCount - 1; i >= 0; i--) {
        PCallFrame *frame = &vm->frames[i];
        struct OComFunction *fn = &frame->f->v.OComFunction;
        PanPrint("in ");
        if (fn->strName != NULL) {
            struct OString *name = &fn->strName->v.OString;
            PanPrint("%s(...)\n", name->value);
        } else {
            PanPrint("<script>\n");
            ;
        }
    }
}

static void vmError(PVm *vm, const char *msg) {
    PanPrint("Error occured:\n");
    vmPrintStackTrace(vm);
    CoreRuntimeError(
        vm->core, vm->frames[0].f->v.OComFunction.code->tokens[0], msg
    );
}

static bool vmBinaryOpNumber(PVm *vm, PanOpCode op, PValue left, PValue right) {
    double leftVal = ValueAsNum(left);
    double rightVal = ValueAsNum(right);
    double result = 0.0;
    switch (op) {
        case OP_ADD: result = leftVal + rightVal; break;
        case OP_SUB: result = leftVal - rightVal; break;
        case OP_MUL: result = leftVal * rightVal; break;
        case OP_DIV: {
            if (rightVal == 0.0) {
                vmError(vm, "Divison by zero");
                return false;
            }
            result = leftVal / rightVal;
            break;
        }
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
        return false;
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
            vmError(vm, "Failed to binary operation");
            return false;
        }
    } else if (IsValueObjType(left, OT_STR) && IsValueObjType(right, OT_STR)) {
        bool isok = vmBinaryOpString(vm, op, left, right);
        if (!isok) {
            PanPrint("Failed to binary operation on string\n");
            return false; // todo handle better
        }
    } else {
        vmError(vm, "Invalid Binary Operation");
        return false;
    }

    return true;
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
        // PanPrint("Invalid Compare Operation!\n");
        CoreRuntimeError(vm->core, NULL, "Invalid Compare Operation");
        return false;
    }
}

static bool vmCallFunction(PVm *vm, PObj *funcObj, int argCount) {
    if (funcObj->v.OComFunction.paramCount != (u64)argCount) {
        PanPrint("Function call argument count != function param count\n");
        return false;
    }
    PCallFrame *frame = &vm->frames[vm->frameCount++];
    frame->f = funcObj;
    frame->ip = funcObj->v.OComFunction.code->code;
    frame->slots = vm->sp - argCount - 1;
    return true;
}

static bool vmCallValue(PVm *vm, PValue callee, int argCount) {
    if (IsValueObjType(callee, OT_COMFNC)) {
        return vmCallFunction(vm, ValueAsObj(callee), argCount);
    }

    return false;
}

static finline u8 vmReadByte(PVm *vm, PCallFrame *frame) {
    return *frame->ip++;
}

static finline u16 vmReadU16(PVm *vm, PCallFrame *frame) {
    frame->ip += 2;
    return ((u16)(frame->ip[-2] << 8) | (u16)(frame->ip[-1]));
}

static finline PValue vmReadConst(PVm *vm, PCallFrame *frame) {
    return frame->f->v.OComFunction.code->constPool[vmReadU16(vm, frame)];
}

static finline PObj *vmReadObjConst(PVm *vm, PCallFrame *frame) {
    return ValueAsObj(
        frame->f->v.OComFunction.code->constPool[vmReadU16(vm, frame)]
    );
}

void VmRun(PVm *vm) {
    PCallFrame *frame = &vm->frames[vm->frameCount - 1];
    while (true) {
        u8 ins;

        switch (ins = vmReadByte(vm, frame)) {
            case OP_RETURN: {
                PValue result = vmPop(vm);
                vm->frameCount--;
                if (vm->frameCount == 0) {
                    vmPop(vm);
                    return;
                }
                vm->sp = frame->slots;
                vmPush(vm, result);
                frame = &vm->frames[vm->frameCount - 1];
                break;
            }
            case OP_DEBUG: {
                PrintValue(vmPop(vm));
                PanPrint("\n");
                break;
            }

            case OP_CONST: {
                PValue val = vmReadConst(vm, frame);
                vmPush(vm, val);
                break;
            }
            case OP_POP: {
                vmPop(vm);
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
                break;
            }
            case OP_DEFINE_GLOBAL: {
                PObj *nameObj = vmReadObjConst(vm, frame);
                if (nameObj->type != OT_STR) {
                    break;
                }

                SymbolTableSet(vm->globals, nameObj, vmPeek(vm, 0));
                vmPop(vm);
                break;
            }
            case OP_GET_GLOBAL: {
                PObj *nameObj = vmReadObjConst(vm, frame);
                bool found = false;
                PValue val = SymbolTableFind(vm->globals, nameObj, &found);
                if (!found) {
                    PanPrint(
                        "Undefined Variable Found -> %s\n",
                        nameObj->v.OString.value
                    );
                    return;
                }

                vmPush(vm, val);
                break;
            }

            case OP_SET_GLOBAL: {
                PObj *nameObj = vmReadObjConst(vm, frame);
                bool found = SymbolTableHasKey(vm->globals, nameObj);
                if (found) {
                    SymbolTableSet(vm->globals, nameObj, vmPeek(vm, 0));
                    break;
                } else {
                    PanPrint(
                        "Undefined Variable Assignment -> %s\n",
                        nameObj->v.OString.value
                    );
                    return;
                }
                break;
            }

            case OP_GET_LOCAL: {
                u16 localStackIndex = vmReadU16(vm, frame);
                vmPush(vm, frame->slots[localStackIndex]);
                break;
            }
            case OP_SET_LOCAL: {
                u16 localStackSlot = vmReadU16(vm, frame);
                frame->slots[localStackSlot] = vmPeek(vm, 0);
                break;
            }

            case OP_JUMP_IF_FALSE: {
                u16 offset = vmReadU16(vm, frame);
                if (!IsValueTruthy(vmPeek(vm, 0))) {
                    frame->ip += offset;
                }
                break;
            }
            case OP_JUMP: {
                u16 offset = vmReadU16(vm, frame);
                frame->ip += offset;
                break;
            }

            case OP_POP_JUMP_IF_FALSE: {
                u16 offset = vmReadU16(vm, frame);
                if (!IsValueTruthy(vmPeek(vm, 0))) {
                    frame->ip += offset;
                } else {
                    vmPop(vm);
                }
                break;
            }

            case OP_POP_JUMP_IF_TRUE: {
                u16 offset = vmReadU16(vm, frame);
                if (IsValueTruthy(vmPeek(vm, 0))) {
                    frame->ip += offset;
                } else {
                    vmPop(vm);
                }
                break;
            }

            case OP_LOOP: {
                u16 offset = vmReadU16(vm, frame);
                frame->ip -= offset;
                break;
            }
            case OP_CALL: {
                u16 argCount = vmReadU16(vm, frame);
                PValue callee = vmPeek(vm, argCount);
                if (!IsValueObjType(callee, OT_COMFNC)) {
                    PanPrint("Can only call functions");
                    return;
                }

                if (vm->frameCount >= PVM_FRAMESTACK_SIZE) {
                    PanPrint("Call stack overflow");
                    return;
                }

                if (!vmCallValue(vm, callee, argCount)) {
                    PanPrint("Failed to call function");
                    return;
                }

                frame = &vm->frames[vm->frameCount - 1];
                break;
            }
        }

        // PanPrint("== %s ==\n", GetOpDefinition(ins).name);
        // DebugVMStack(vm);
    }
}
