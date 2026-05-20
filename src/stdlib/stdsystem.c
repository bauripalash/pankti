#include "../include/alloc.h"
#include "../include/gc.h"
#include "../include/pstdlib.h"
#include "../include/system.h"
#include "../include/vm.h"
#include <stdbool.h>

#define SYS_NAME_LINUX  "লিনাক্স"
#define SYS_NAME_WIN    "উইন্ডোজ"
#define SYS_NAME_MAC    "ম্যাকওএস"

#define SYS_ARCH_ARM    "আর্ম"
#define SYS_ARCH_X86    "এক্স৮৬"
#define SYS_ARCH_X86_64 "এক্স৮৬_৬৪"

#define SYS_WEB         "ওয়েব"
#define SYS_UNKNOWN     "অজানা"

static PValue os_Name(PVm *vm, PValue *args, u64 argc) {
#if defined(PANKTI_OS_LINUX)
    char *name = SYS_NAME_LINUX;
#elif defined(PANKTI_OS_WIN)
    char *name = SYS_NAME_WIN;
#elif defined(PANKTI_OS_MAC)
    char *name = SYS_NAME_MAC;
#elif defined(PANKTI_SYS_WEB)
    char *name = SYS_WEB;
#else
    char *name = SYS_UNKNOWN;
#endif
    PObj *nameStrObj = NewStrObject(vm->gc, NULL, name, false);
    if (nameStrObj == NULL) {
        VmError(vm, RT_IME_STDSYS_NAME_STR);
        return MakeNil();
    }
    return MakeObject(nameStrObj);
}
static PValue os_Arch(PVm *vm, PValue *args, u64 argc) {
#if defined(PANKTI_ARCH_X86_64)
    char *arch = SYS_ARCH_X86_64;
#elif defined(PANKTI_ARCH_X86)
    char *arch = SYS_ARCH_X86;
#elif defined(PANKTI_ARCH_ARM)
    char *arch = SYS_ARCH_ARM;
#elif defined(PANKTI_SYS_WEB)
    char *arch = SYS_WEB;
#else
    char *arch = SYS_UNKNOWN;
#endif
    PObj *archStrObj = NewStrObject(vm->gc, NULL, arch, false);
    if (archStrObj == NULL) {
        VmError(vm, RT_IME_STDSYS_ARCH_STR);
        return MakeNil();
    }
    return MakeObject(archStrObj);
}
static PValue os_Username(PVm *vm, PValue *args, u64 argc) {
#if defined(PANKTI_OS_WEB)
    char *username = SYS_WEB;
#else
    char *username = GetOsUsername();
#endif
    if (username != NULL) {
        PObj *usernameStrObj = NewStrObject(vm->gc, NULL, username, false);
        if (usernameStrObj == NULL) {
            VmError(vm, RT_IME_STDSYS_USERNAME_STR);
            return MakeNil();
        }
        return MakeObject(usernameStrObj);
    }
    PObj *unknownUserStrObj = NewStrObject(vm->gc, NULL, SYS_UNKNOWN, false);
    if (unknownUserStrObj == NULL) {
        VmError(vm, RT_IME_STDSYS_USERNAME_STR);
        return MakeNil();
    }
    return MakeObject(unknownUserStrObj);
}
static PValue os_HomeDir(PVm *vm, PValue *args, u64 argc) {
#if defined(PANKTI_OS_WEB)
    char *homedir = SYS_WEB;
#else
    char *homedir = GetHomeDir();
#endif
    if (homedir != NULL) {
        PObj *homeStrObj = NewStrObject(vm->gc, NULL, homedir, false);
        if (homeStrObj == NULL) {
            VmError(vm, RT_IME_STDSYS_HOMEDIR_STR);
            return MakeNil();
        }
        return MakeObject(homeStrObj);
    }
    PObj *unknownStrObj = NewStrObject(vm->gc, NULL, SYS_UNKNOWN, false);
    if (unknownStrObj == NULL) {
        VmError(vm, RT_IME_STDSYS_HOMEDIR_STR);
        return MakeNil();
    }
    return MakeObject(unknownStrObj);
}
static PValue os_CurDir(PVm *vm, PValue *args, u64 argc) {
#if defined(PANKTI_OS_WEB)
    char *curdir = SYS_WEB;
#else
    char *curdir = GetCurDir();
#endif
    if (curdir != NULL) {
        PObj *curDirStrObj = NewStrObject(vm->gc, NULL, curdir, false);
        PFree(curdir);
        if (curDirStrObj == NULL) {
            VmError(vm, RT_IME_STDSYS_CURDIR_STR);
            return MakeNil();
        }
        return MakeObject(curDirStrObj);
    }
    PObj *unknownStrObj = NewStrObject(vm->gc, NULL, SYS_UNKNOWN, false);
    if (unknownStrObj == NULL) {
        VmError(vm, RT_IME_STDSYS_CURDIR_STR);
        return MakeNil();
    }
    return MakeObject(unknownStrObj);
}

#define OS_STD_NAME     "নাম"
#define OS_STD_ARCH     "আর্চ"
#define OS_STD_USERNAME "ব্যবহারকারী"
#define OS_STD_HOMEDIR  "ঘর"
#define OS_STD_CURDIR   "অবস্থান"

void PushStdlibSystem(PVm *vm, SymbolTable *table) {
    StdlibEntry entries[] = {
        MakeStdlibEntry(OS_STD_NAME, os_Name, 0),
        MakeStdlibEntry(OS_STD_ARCH, os_Arch, 0),
        MakeStdlibEntry(OS_STD_USERNAME, os_Username, 0),
        MakeStdlibEntry(OS_STD_HOMEDIR, os_HomeDir, 0),
        MakeStdlibEntry(OS_STD_CURDIR, os_CurDir, 0),
    };

    int count = ArrCount(entries);
    PushStdlibEntries(vm, table, SYSTEM_STDLIB_NAME, entries, count);
}
