#include "../include/env.h"
#include "../include/interpreter.h"
#include "../include/pstdlib.h"
#include <stdbool.h>

#define OS_NAME_LINUX "লিনাক্স"
#define OS_NAME_WIN   "উইন্ডোজ"
#define OS_NAME_MAC   "ম্যাকওএস"

#define OS_ARCH_ARM "আর্ম"
#define OS_ARCH_X86 "এক্স86"
#define OS_ARCH_X86_64 "এক্স86_64"

#define OS_WEB   "ওয়েব"
#define OS_UNKNOWN    "অজানা"

static PValue os_Name(PInterpreter *it, PValue *args, u64 argc) {
#if defined(PANKTI_OS_LINUX)
    char *name = OS_NAME_LINUX;
#elif defined(PANKTI_OS_WIN)
    char *name = OS_NAME_WIN;
#elif defined(PANKTI_OS_MAC)
    char *name = OS_NAME_MAC;
#elif defined(PANKTI_OS_WEBB)
    char *name = OS_WEB;
#else
    char *name = OS_UNKNOWN;
#endif
	PObj * nameStrObj = NewStrObject(it->gc, NULL, StrDuplicate(name, StrLength(name)) , true);
	return MakeObject(nameStrObj);

}
static PValue os_Arch(PInterpreter *it, PValue *args, u64 argc){
	return MakeNil();
}
static PValue os_Username(PInterpreter *it, PValue *args, u64 argc){
	return MakeNil();
}
static PValue os_HomeDir(PInterpreter *it, PValue *args, u64 argc){
	return MakeNil();
}
static PValue os_CurDir(PInterpreter *it, PValue *args, u64 argc){
	return MakeNil();
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
