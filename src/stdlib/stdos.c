#include "../include/alloc.h"
#include "../include/interpreter.h"
#include "../include/pstdlib.h"
#include "../include/system.h"
#include <stdbool.h>

#define OS_NAME_LINUX  "লিনাক্স"
#define OS_NAME_WIN    "উইন্ডোজ"
#define OS_NAME_MAC    "ম্যাকওএস"

#define OS_ARCH_ARM    "আর্ম"
#define OS_ARCH_X86    "এক্স৮৬"
#define OS_ARCH_X86_64 "এক্স৮৬_৬৪"

#define OS_WEB         "ওয়েব"
#define OS_UNKNOWN     "অজানা"

static PValue os_Name(PInterpreter *it, PValue *args, u64 argc) {
#if defined(PANKTI_OS_LINUX)
    char *name = OS_NAME_LINUX;
#elif defined(PANKTI_OS_WIN)
    char *name = OS_NAME_WIN;
#elif defined(PANKTI_OS_MAC)
    char *name = OS_NAME_MAC;
#elif defined(PANKTI_OS_WEB)
    char *name = OS_WEB;
#else
    char *name = OS_UNKNOWN;
#endif
    PObj *nameStrObj =
        NewStrObject(it->gc, NULL, StrDuplicate(name, StrLength(name)), true);
    return MakeObject(nameStrObj);
}
static PValue os_Arch(PInterpreter *it, PValue *args, u64 argc) {
#if defined(PANKTI_ARCH_X86_64)
    char *arch = OS_ARCH_X86_64;
#elif defined(PANKTI_ARCH_X86)
    char *arch = OS_ARCH_X86;
#elif defined(PANKTI_ARCH_ARM)
    char *arch = OS_ARCH_ARM;
#elif defined(PANKTI_OS_WEB)
    char *arch = OS_WEB;
#else
    char *arch = OS_UNKNOWN;
#endif
    PObj *archStrObj =
        NewStrObject(it->gc, NULL, StrDuplicate(arch, StrLength(arch)), true);
    return MakeObject(archStrObj);
}
static PValue os_Username(PInterpreter *it, PValue *args, u64 argc) {
#if defined(PANKTI_OS_WEB)
    char *username = OS_WEB;
#else
    char *username = GetOsUsername();
#endif
    if (username != NULL) {
        PObj *usernameStrObj = NewStrObject(
            it->gc, NULL, StrDuplicate(username, StrLength(username)), true
        );
        return MakeObject(usernameStrObj);
    }
    PObj *unknownUserStrObj = NewStrObject(
        it->gc, NULL, StrDuplicate(OS_UNKNOWN, StrLength(OS_UNKNOWN)), true
    );
    return MakeObject(unknownUserStrObj);
}
static PValue os_HomeDir(PInterpreter *it, PValue *args, u64 argc) {
#if defined(PANKTI_OS_WEB)
    char *homedir = OS_WEB;
#else
    char *homedir = GetHomeDir();
#endif
    if (homedir != NULL) {
        PObj *usernameStrObj = NewStrObject(
            it->gc, NULL, StrDuplicate(homedir, StrLength(homedir)), true
        );
        return MakeObject(usernameStrObj);
    }
    PObj *unknownUserStrObj = NewStrObject(
        it->gc, NULL, StrDuplicate(OS_UNKNOWN, StrLength(OS_UNKNOWN)), true
    );
    return MakeObject(unknownUserStrObj);
}
static PValue os_CurDir(PInterpreter *it, PValue *args, u64 argc) {
#if defined(PANKTI_OS_WEB)
    char *curdir = OS_WEB;
#else
    char *curdir = GetCurDir();
#endif
    if (curdir != NULL) {
        PObj *usernameStrObj = NewStrObject(
            it->gc, NULL, StrDuplicate(curdir, StrLength(curdir)), true
        );
        PFree(curdir);
        return MakeObject(usernameStrObj);
    }
    PObj *unknownUserStrObj = NewStrObject(
        it->gc, NULL, StrDuplicate(OS_UNKNOWN, StrLength(OS_UNKNOWN)), true
    );
    return MakeObject(unknownUserStrObj);
}

#define OS_STD_NAME     "নাম"
#define OS_STD_ARCH     "আর্চ"
#define OS_STD_USERNAME "ব্যবহারকারী"
#define OS_STD_HOMEDIR  "ঘর"
#define OS_STD_CURDIR   "বর্তমান"

void PushStdlibOs(PInterpreter *it, PEnv *env) {
    StdlibEntry entries[] = {
        MakeStdlibEntry(OS_STD_NAME, os_Name, 0),
        MakeStdlibEntry(OS_STD_ARCH, os_Arch, 0),
        MakeStdlibEntry(OS_STD_USERNAME, os_Username, 0),
        MakeStdlibEntry(OS_STD_HOMEDIR, os_HomeDir, 0),
        MakeStdlibEntry(OS_STD_CURDIR, os_CurDir, 0),
    };

    int count = ArrCount(entries);
    for (int i = 0; i < count; i++) {
        const StdlibEntry *entry = &entries[i];
        PObj *stdFn = NewNativeFnObject(it->gc, NULL, entry->fn, entry->arity);
        EnvPutValue(
            env, StrHash(entry->name, entry->nlen, it->gc->timestamp),
            MakeObject(stdFn)
        );
    }
}
