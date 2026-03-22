#include "../include/gc.h"
#include "../include/pstdlib.h"
#include "../include/utils.h"
#include "../include/vm.h"
#include <stdbool.h>

static PValue file_Exists(PVm *vm, PValue *args, u64 argc) {
    PValue rawFilePath = args[0];
    if (!IsValueObjType(rawFilePath, OT_STR)) {
        VmError(vm, "File path must be string");
        return MakeNil();
    }

    char *filePathStr = ValueAsObj(rawFilePath)->v.OString.value;
    bool result = DoesFileExists(filePathStr);

    return MakeBool(result);
}

static PValue file_ReadAsString(PVm *vm, PValue *args, u64 argc) {
    PValue rawFilePath = args[0];
    if (!IsValueObjType(rawFilePath, OT_STR)) {
        VmError(vm, "File path must be string");
        return MakeNil();
    }

    char *filePathStr = ValueAsObj(rawFilePath)->v.OString.value;
    if (!DoesFileExists(filePathStr)) {
        const char *err = StrFormat("'%s' file doesn't exist", filePathStr);
        VmError(vm, (char *)err);
        return MakeNil();
    }

    if (!PanIsPathFile(filePathStr)) {
        const char *err = StrFormat("'%s' is not a file", filePathStr);
        VmError(vm, (char *)err);
        return MakeNil();
    }

    char *fileStr = PanReadFile(filePathStr);
    if (fileStr == NULL) {
        const char *err = StrFormat("failed to read file '%s' ", filePathStr);
        VmError(vm, (char *)err);
        return MakeNil();
    }

    PObj *strObj = NewStrObject(vm->gc, NULL, fileStr);
    if (strObj == NULL) {
        const char *err = StrFormat(
            "failed to create string while reading file '%s' ", filePathStr
        );
        VmError(vm, (char *)err);
        return MakeNil();
    }
    return MakeObject(strObj);
}

static PValue file_WriteStringToFile(PVm *vm, PValue *args, u64 argc) {
    PValue rawFilePath = args[0];
    PValue rawContent = args[1];
    if (!IsValueObjType(rawFilePath, OT_STR)) {
        VmError(vm, "File path must be string");
        return MakeNil();
    }

    if (!IsValueObjType(rawContent, OT_STR)) {
        VmError(vm, "File content must be a string");
        return MakeNil();
    }

    char *filePathStr = ValueAsObj(rawFilePath)->v.OString.value;
    if (!DoesFileExists(filePathStr)) {
        const char *err = StrFormat("'%s' file doesn't exist", filePathStr);
        VmError(vm, (char *)err);
        return MakeNil();
    }

    if (!PanIsPathFile(filePathStr)) {
        const char *err = StrFormat("'%s' is not a file", filePathStr);
        VmError(vm, (char *)err);
        return MakeNil();
    }

    char *contentStr = ValueAsObj(rawContent)->v.OString.value;
    if (!PanWriteFile(filePathStr, contentStr)) {
        const char *err = StrFormat("failed to write file '%s' ", filePathStr);
        VmError(vm, (char *)err);
        return MakeNil();
    }
    return MakeNil();
}

static PValue file_CreateFile(PVm *vm, PValue *args, u64 argc) {
    PValue rawFilePath = args[0];

    if (!IsValueObjType(rawFilePath, OT_STR)) {
        VmError(vm, "File path must be string");
        return MakeNil();
    }

    char *filePathStr = ValueAsObj(rawFilePath)->v.OString.value;
    PanCreateFile(filePathStr);
    return MakeNil();
}

static PValue file_CreateDir(PVm *vm, PValue *args, u64 argc) {
    PValue rawFilePath = args[0];

    if (!IsValueObjType(rawFilePath, OT_STR)) {
        VmError(vm, "Directory path must be string");
        return MakeNil();
    }

    char *filePathStr = ValueAsObj(rawFilePath)->v.OString.value;
    PanCreateDir(filePathStr);
    return MakeNil();
}

#define FILE_STD_EXISTS      "exists"
#define FILE_STD_READ        "read"
#define FILE_STD_WRITE       "write"
#define FILE_STD_CREATE_FILE "newfile"
#define FILE_STD_CREATE_DIR  "newdir"

void PushStdlibFile(PVm *vm, SymbolTable *table) {
    StdlibEntry entries[] = {
        MakeStdlibEntry(FILE_STD_EXISTS, file_Exists, 1),
        MakeStdlibEntry(FILE_STD_READ, file_ReadAsString, 1),
        MakeStdlibEntry(FILE_STD_WRITE, file_WriteStringToFile, 2),
        MakeStdlibEntry(FILE_STD_CREATE_FILE, file_CreateFile, 1),
        MakeStdlibEntry(FILE_STD_CREATE_DIR, file_CreateDir, 1),
    };
    int count = ArrCount(entries);

    PushStdlibEntries(vm, table, MAP_STDLIB_NAME, entries, count);
}
