#include "include/vm.h"
#include "external/stb/stb_ds.h"
#include "include/alloc.h"
#include "include/compiler.h"
#include "include/core.h"
#include "include/gc.h"
#include "include/native.h"
#include "include/object.h"
#include "include/opcode.h"
#include "include/printer.h"
#include "include/pstdlib.h"
#include "include/ptypes.h"
#include "include/symtable.h"
#include "include/utils.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

PVm *NewVm(PanktiCore *core) {
    PVm *vm = PCreate(PVm);
    vm->core = core;
    vm->sp = vm->stack;
    vm->gc = core->gc;
    vm->frameCount = 0;
    vm->globals = NewSymbolTable();
    vm->modCount = 0;
    vm->modules = NULL;
    vm->modProxiesCount = 0;
    vm->modProxies = NULL;
    vm->sp = vm->stack;
    return vm;
}

void SetupVm(PVm *vm, Pgc *gc, PObj *func) {
    vm->gc = gc;
    VmPush(vm, MakeObject(func));
    PCallFrame *frame = &vm->frames[vm->frameCount++];
    frame->f = func;
    frame->ip = func->v.OComFunction.code->code;
    frame->slots = vm->stack;
    RegisterNatives(vm, NULL);
}
void FreeVm(PVm *vm) {
    if (vm == NULL) {
        return;
    }

    FreeSymbolTable(vm->globals);

    if (vm->modProxiesCount > 0 && vm->modProxies != NULL) {
        hmfree(vm->modProxies);
    }

    if (vm->modCount > 0 && vm->modules != NULL) {
        for (int i = 0; i < vm->modCount; i++) {
            PModule *mod = arrpop(vm->modules);
            PFree(mod->pathname);
            FreeSymbolTable(mod->table);
            PFree(mod);
        }

        arrfree(vm->modules);
    }

    PFree(vm);
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

static PModule *NewModule(PVm *vm, char *name) {
    PModule *mod = PCreate(PModule);

    if (mod == NULL) {
        return NULL;
    }

    mod->pathname = StrDuplicate(name, StrLength(name));
    mod->table = NewSymbolTable();

    return mod;
}

static void PushModule(PVm *vm, PModule *mod) {
    arrput(vm->modules, mod);
    vm->modCount = (u64)arrlen(vm->modules);
}

static void PushProxy(PVm *vm, u64 key, char *name, PModule *mod) {
    if (mod == NULL) {
        return; // error check
    }

    ModProxyEntry s = {.key = key, .name = name, .mod = mod};
    hmputs(vm->modProxies, s);
    vm->modProxiesCount = (u64)hmlen(vm->modProxies);
}

void DebugVMStack(PVm *vm) {
    PanPrint("==== STACK ====\n");
    int i = vm->sp - vm->stack - 1;
    for (PValue *val = vm->stack; val < vm->sp; val++) {
        PanPrint("[ |%d| ", i);
        PrintValue(*val);
        PanPrint(" ]\n");
        i--;
    }
    PanPrint("== END STACK ==\n");
}

PValue VmGetLastPopped(const PVm *vm) { return MakeNil(); }

bool VmPush(PVm *vm, PValue val) {
    if (vm->sp >= vm->stack + PVM_STACK_SIZE) {
        vmError(vm, "Pankti VM Stack Overflow!");
        return false;
    }
    *vm->sp = val;
    vm->sp++;
    return true;
}

PValue VmPop(PVm *vm) {
    if (vm->sp <= vm->stack) {
        vmError(vm, "Pankti VM Stack Undeflow!");
        return MakeNil();
    }
    vm->sp--;
    return *vm->sp;
}

PValue VmPeek(const PVm *vm, int index) {

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
    VmPop(vm);
    VmPop(vm);
    return VmPush(vm, MakeNumber(result));
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
    VmPop(vm);
    VmPop(vm);

    return VmPush(vm, MakeObject(nsObj));
}

static bool vmBinaryOp(PVm *vm, PanOpCode op) {
    PValue right = VmPeek(vm, 0);
    PValue left = VmPeek(vm, 1);

    if (IsValueNum(left) && IsValueNum(right)) {
        bool isok = vmBinaryOpNumber(vm, op, left, right);
        if (!isok) {
            vmError(vm, "Failed to binary operation");
            return false;
        }
    } else if (IsValueObjType(left, OT_STR) && IsValueObjType(right, OT_STR)) {
        bool isok = vmBinaryOpString(vm, op, left, right);
        if (!isok) {
            vmError(vm, "Failed to binary operation on string");
            return false; // todo handle better
        }
    } else {
        vmError(vm, "Invalid Binary Operation");
        return false;
    }

    return true;
}

static bool vmCompareOp(PVm *vm, PanOpCode op) {
    PValue rightVal = VmPeek(vm, 0);
    PValue leftVal = VmPeek(vm, 1);

    if (IsValueNum(leftVal) && IsValueNum(rightVal)) {
        double left = ValueAsNum(leftVal);
        double right = ValueAsNum(rightVal);
        bool result = false;
        switch (op) {
            case OP_GT: result = left > right; break;
            case OP_GTE: result = left >= right; break;
            case OP_LT: result = left < right; break;
            case OP_LTE: result = left <= right; break;
            default: break; // should never reach here
        }

        VmPop(vm);
        VmPop(vm);
        VmPush(vm, MakeBool(result));
        return true;
    } else {
        vmError(vm, "Invalid Compare Operation");
        return false;
    }
}

static bool vmCallFunction(PVm *vm, PObj *funcObj, int argCount) {
    if (funcObj->v.OComFunction.paramCount != (u64)argCount) {
        vmError(vm, "Function call argument count != function param count");
        return false;
    }
    PCallFrame *frame = &vm->frames[vm->frameCount++];
    frame->f = funcObj;
    frame->ip = funcObj->v.OComFunction.code->code;
    frame->slots = vm->sp - argCount - 1;
    return true;
}

static bool vmCallNative(PVm *vm, PObj *funcObj, int argc) {
    struct ONative *native = &funcObj->v.ONative;
    if (native->arity != -1 && native->arity != argc) {
        vmError(vm, "Native function arg count != param count");
        return false;
    }

    PValue result = native->fn(vm, vm->sp - argc, argc);
    vm->sp -= argc + 1;
    VmPush(vm, result);
    if (IsValueObjType(result, OT_ERROR)) {
        vmError(vm, ValueAsObj(result)->v.OError.msg);
        return false;
    }
    return true;
}

static bool vmCallValue(PVm *vm, PValue callee, int argCount) {
    if (IsValueObjType(callee, OT_COMFNC)) {
        return vmCallFunction(vm, ValueAsObj(callee), argCount);
    } else if (IsValueObjType(callee, OT_NATIVE)) {
        return vmCallNative(vm, ValueAsObj(callee), argCount);
    }

    return false;
}

static finline PValue vmGetSubIndex(PVm *vm, bool assign) {
    if (assign) {
        return VmPeek(vm, 1);
    } else {
        return VmPeek(vm, 0);
    }
}

static bool vmArraySubscript(PVm *vm, PValue target, bool assign) {

    PValue indexVal = vmGetSubIndex(vm, assign);

    if (!IsValueNum(indexVal)) {
        vmError(vm, "Arrays can only be indexed with non-negetive integers");
        return false;
    }

    double dblIndex = ValueAsNum(indexVal);
    if (dblIndex < 0 || !IsDoubleInt(dblIndex)) {
        vmError(vm, "Arrays can only be indexed with non-negetive integers");
        return false;
    }
    u64 index = (u64)floor(dblIndex);

    struct OArray *arrObj = &ValueAsObj(target)->v.OArray;

    if (index >= arrObj->count) {
        vmError(vm, "Arrays index out of range");
        return false;
    }

    PValue result = MakeNil();

    if (assign) {
        result = VmPeek(vm, 0);
        arrObj->items[index] = result;
        VmPop(vm); // new value
        VmPop(vm); // index
        VmPop(vm); // target
    } else {
        result = arrObj->items[index];
        VmPop(vm); // index
        VmPop(vm); // target
    }

    VmPush(vm, result);
    return true;
}

static bool vmMapSubscript(PVm *vm, PValue target, bool assign) {
    PValue keyVal = vmGetSubIndex(vm, assign);

    if (!CanValueBeKey(keyVal)) {
        vmError(vm, "Subscript key is invalid as a map key");
        return false;
    }

    u64 keyHash = GetValueHash(keyVal, vm->gc->timestamp);
    PObj *mapObj = ValueAsObj(target);
    PValue result;
    if (assign) {
        PValue newValue = VmPeek(vm, 0);
        MapObjSetValue(mapObj, keyVal, keyHash, newValue);
        VmPop(vm); // new value
        VmPop(vm); // key
        VmPop(vm); // target : map
        result = newValue;
    } else {
        bool found = false;
        result = MapObjGetValue(ValueAsObj(target), keyVal, keyHash, &found);
        if (!found) {
            vmError(vm, "Key doesn't exist in map");
            return false;
        }
        VmPop(vm); // key
        VmPop(vm); // target
    }

    VmPush(vm, result);
    return true;
}

static bool vmSubscript(PVm *vm) {
    PValue targetVal = VmPeek(vm, 1);

    if (IsValueObjType(targetVal, OT_ARR)) {
        return vmArraySubscript(vm, targetVal, false);
    } else if (IsValueObjType(targetVal, OT_MAP)) {
        return vmMapSubscript(vm, targetVal, false);
    } else {
        vmError(vm, "Invalid Subscript target");
        return false;
    }
}

static bool vmSubscriptAssign(PVm *vm) {
    PValue targetVal = VmPeek(vm, 2);

    if (IsValueObjType(targetVal, OT_ARR)) {
        return vmArraySubscript(vm, targetVal, true);
    } else if (IsValueObjType(targetVal, OT_MAP)) {
        return vmMapSubscript(vm, targetVal, true);
    } else {
        vmError(vm, "Invalid Subscript Assignment target");
        return false;
    }
}

static bool vmImportModule(PVm *vm, PValue name) {
    PValue importPath = VmPeek(vm, 0);
    if (!IsValueObjType(importPath, OT_STR)) {
        vmError(vm, "Import Path must be a string");
        return false;
    }

    char *pathStr = ValueAsObj(importPath)->v.OString.value;

    StdlibMod stdmod = GetStdlibMod(pathStr);
    if (stdmod == STDLIB_NONE) {
        vmError(vm, "Currently only stdlib imports are supported");
        return false;
    }

    PModule *mod = NewModule(vm, pathStr);

    if (mod == NULL) {
        vmError(vm, "Failed to create module");
        return false;
    }

    PushModule(vm, mod);
    PObj *nameObj = ValueAsObj(name);
    u64 key = nameObj->v.OString.hash;
    PushProxy(vm, key, nameObj->v.OString.name->lexeme, mod);

    PushStdlib(vm, mod->table, mod->pathname, stdmod);

    PObj *modObject =
        NewModuleObject(vm->gc, nameObj->v.OString.value, pathStr);
    SymbolTableSet(vm->globals, nameObj, MakeObject(modObject));
    VmPop(vm); // remove the importPath

    return true;
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
                PValue result = VmPop(vm);
                vm->frameCount--;
                if (vm->frameCount == 0) {
                    VmPop(vm);
                    return;
                }
                vm->sp = frame->slots;
                VmPush(vm, result);
                frame = &vm->frames[vm->frameCount - 1];
                break;
            }
            case OP_DEBUG: {
                PrintValue(VmPop(vm));
                PanPrint("\n");
                break;
            }

            case OP_CONST: {
                PValue val = vmReadConst(vm, frame);
                VmPush(vm, val);
                break;
            }
            case OP_POP: {
                VmPop(vm);
                break;
            }
            case OP_TRUE: {
                VmPush(vm, MakeBool(true));
                break;
            }
            case OP_FALSE: {
                VmPush(vm, MakeBool(false));
                break;
            }
            case OP_NIL: {
                VmPush(vm, MakeNil());
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
                PValue b = VmPeek(vm, 0);
                PValue a = VmPeek(vm, 1);
                bool result = IsValueEqual(a, b);
                if (ins == OP_NOTEQUAL) {
                    result = !result;
                }
                VmPop(vm);
                VmPop(vm);
                VmPush(vm, MakeBool(result));
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
                if (!IsValueNum(VmPeek(vm, 0))) {
                    break;
                }

                VmPush(vm, MakeNumber(-ValueAsNum(VmPop(vm))));
                break;
            }

            case OP_NOT: {
                VmPush(vm, MakeBool(!IsValueTruthy(VmPop(vm))));
                break;
            }
            case OP_DEFINE_GLOBAL: {
                PObj *nameObj = vmReadObjConst(vm, frame);
                if (nameObj->type != OT_STR) {
                    break;
                }

                SymbolTableSet(vm->globals, nameObj, VmPeek(vm, 0));
                VmPop(vm);
                break;
            }
            case OP_GET_GLOBAL: {
                PObj *nameObj = vmReadObjConst(vm, frame);
                bool found = false;
                PValue val = SymbolTableFind(vm->globals, nameObj, &found);
                if (!found) {
                    const char *errorMsg = StrFormat(
                        "Undefined variable found : %s",
                        nameObj->v.OString.value
                    );
                    vmError(vm, errorMsg);
                    return;
                }

                VmPush(vm, val);
                break;
            }

            case OP_SET_GLOBAL: {
                PObj *nameObj = vmReadObjConst(vm, frame);
                bool found = SymbolTableHasKey(vm->globals, nameObj);
                if (found) {
                    SymbolTableSet(vm->globals, nameObj, VmPeek(vm, 0));
                    break;
                } else {
                    const char *errorMsg = StrFormat(
                        "Undefined Variable Assignment : %s",
                        nameObj->v.OString.value
                    );
                    vmError(vm, errorMsg);
                    return;
                }
                break;
            }

            case OP_GET_LOCAL: {
                u16 localStackIndex = vmReadU16(vm, frame);
                VmPush(vm, frame->slots[localStackIndex]);
                break;
            }
            case OP_SET_LOCAL: {
                u16 localStackSlot = vmReadU16(vm, frame);
                frame->slots[localStackSlot] = VmPeek(vm, 0);
                break;
            }

            case OP_JUMP_IF_FALSE: {
                u16 offset = vmReadU16(vm, frame);
                if (!IsValueTruthy(VmPeek(vm, 0))) {
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
                if (!IsValueTruthy(VmPeek(vm, 0))) {
                    frame->ip += offset;
                } else {
                    VmPop(vm);
                }
                break;
            }

            case OP_POP_JUMP_IF_TRUE: {
                u16 offset = vmReadU16(vm, frame);
                if (IsValueTruthy(VmPeek(vm, 0))) {
                    frame->ip += offset;
                } else {
                    VmPop(vm);
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
                PValue callee = VmPeek(vm, argCount);
                if (!IsValueObjType(callee, OT_COMFNC) &&
                    !IsValueObjType(callee, OT_NATIVE)) {
                    vmError(vm, "Can only call functions");
                    return;
                }

                if (vm->frameCount >= PVM_FRAMESTACK_SIZE) {
                    vmError(vm, "Call stack overflow");
                    return;
                }

                if (!vmCallValue(vm, callee, argCount)) {
                    vmError(vm, "Failed to call function");

                    return;
                }

                frame = &vm->frames[vm->frameCount - 1];
                break;
            }

            case OP_ARRAY: {
                u16 itemCount = vmReadU16(vm, frame);
                PValue *items = NULL;
                if (itemCount > 0) {
                    arrsetlen(items, itemCount);
                }
                for (u16 i = itemCount; i > 0; i--) {
                    items[i - 1] = VmPop(vm);
                }
                PObj *arrObj = NewArrayObject(vm->gc, NULL, items, itemCount);
                if (arrObj == NULL) {
                    arrfree(items);
                    vmError(vm, "Internal Error : Failed to create array");
                    return;
                }
                VmPush(vm, MakeObject(arrObj));
                break;
            }

            case OP_MAP: {
                u16 pairCount = vmReadU16(vm, frame);
                MapEntry *entries = NULL;
                u64 stackItems = pairCount * 2;
                for (u64 i = stackItems - 2; i >= 0 && i < stackItems; i -= 2) {
                    PValue key = VmPeek(vm, i + 1);
                    PValue val = VmPeek(vm, i);

                    if (!CanValueBeKey(key)) {
                        hmfree(entries);
                        PrintValue(key);
                        vmError(vm, "value cannot be a key");
                        return;
                    }

                    u64 keyHash = GetValueHash(key, vm->gc->timestamp);
                    hmputs(entries, ((MapEntry){keyHash, key, val}));
                }

                vm->sp -= stackItems;
                PObj *mapObj = NewMapObject(vm->gc, NULL);
                if (mapObj == NULL) {
                    hmfree(entries);
                    vmError(vm, "Internal Error : Failed to create map");
                    return;
                }
                mapObj->v.OMap.count = pairCount;
                mapObj->v.OMap.table = entries;
                VmPush(vm, MakeObject(mapObj));
                break;
            }
            case OP_SUBSCRIPT: {
                vmSubscript(vm);
                break;
            }
            case OP_SUBS_ASSIGN: {
                vmSubscriptAssign(vm);
                break;
            }
            case OP_IMPORT: {
                PValue name = vmReadConst(vm, frame);
                vmImportModule(vm, name);
                break;
            }
            case OP_MODGET: {
                PValue child = vmReadConst(vm, frame);
                PValue moduleVal = VmPeek(vm, 0);
                if (!IsValueObjType(moduleVal, OT_MODULE)) {
                    vmError(vm, "Only modules can provide child values");
                    return;
                }
                if (!IsValueObjType(child, OT_STR)) {
                    vmError(vm, "Invalid module child");
                    return;
                }
                PObj *childObj = ValueAsObj(child);
                PObj *modObj = ValueAsObj(moduleVal);
                u64 nameHash = modObj->v.OModule.nameHash;
                if (hmgeti(vm->modProxies, nameHash) < 0) {
                    vmError(vm, "Unknown module found");
                }

                ModProxyEntry proxy = hmgets(vm->modProxies, nameHash);

                PModule *module = proxy.mod;
                bool found = false;
                PValue childResult =
                    SymbolTableFind(module->table, childObj, &found);
                if (!found) {
                    vmError(vm, "Unknown child for module");
                    break;
                }
                VmPop(vm);

                VmPush(vm, childResult);

                break;
            }
        }
    }
}
