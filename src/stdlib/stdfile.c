#include "../include/gc.h"
#include "../include/pstdlib.h"
#include "../include/utils.h"
#include "../include/vm.h"
#include <stdbool.h>

static PValue file_Exists(PVm *vm, PValue *args, u64 argc) {
    PValue rawFilePath = args[0];
    if (!IsValueObjType(rawFilePath, OT_STR)) {
        VmError(vm, RT_STDFILE_EXISTS_FIRST_STR, ValueTypeToStr(rawFilePath));
        return MakeNil();
    }

    char *filePathStr = ValueAsObj(rawFilePath)->v.OString.value;
    bool result = DoesFileExists(filePathStr);

    return MakeBool(result);
}

static PValue file_ReadAsString(PVm *vm, PValue *args, u64 argc) {
    PValue rawFilePath = args[0];
    if (!IsValueObjType(rawFilePath, OT_STR)) {
        VmError(vm, RT_STDFILE_EXISTS_FIRST_STR, ValueTypeToStr(rawFilePath));
        return MakeNil();
    }

    char *filePathStr = ValueAsObj(rawFilePath)->v.OString.value;
    if (!DoesFileExists(filePathStr)) {
        VmError(vm, RT_STDFILE_READ_FILE_NOT_FOUND, filePathStr);
        return MakeNil();
    }

    if (!PanIsPathFile(filePathStr)) {
        VmError(vm, RT_STDFILE_READ_ISNOT_FILE, filePathStr);
        return MakeNil();
    }

    char *fileStr = PanReadFile(filePathStr);
    if (fileStr == NULL) {
        VmError(vm, RT_IME_STDFILE_FILE_READ_FILE, filePathStr);
        return MakeNil();
    }

    PObj *strObj = NewStrObject(vm->gc, NULL, fileStr, true);
    if (strObj == NULL) {
        VmError(vm, RT_IME_STDFILE_READ_STR);
        return MakeNil();
    }
    return MakeObject(strObj);
}

static PValue file_WriteStringToFile(PVm *vm, PValue *args, u64 argc) {
    PValue rawFilePath = args[0];
    PValue rawContent = args[1];
    if (!IsValueObjType(rawFilePath, OT_STR)) {
        VmError(vm, RT_STDFILE_WRITE_FILENAME_STR, ValueTypeToStr(rawFilePath));
        return MakeNil();
    }

    if (!IsValueObjType(rawContent, OT_STR)) {
        VmError(vm, RT_STDFILE_WRITE_CONTENT_STR, ValueTypeToStr(rawContent));
        return MakeNil();
    }

    char *filePathStr = ValueAsObj(rawFilePath)->v.OString.value;
    if (!DoesFileExists(filePathStr)) {
        VmError(vm, RT_STDFILE_WRITE_FILE_NOT_FOUND, filePathStr);
        return MakeNil();
    }

    if (!PanIsPathFile(filePathStr)) {
        VmError(vm, RT_STDFILE_WRITE_ISNOT_FILE, filePathStr);
        return MakeNil();
    }

    char *contentStr = ValueAsObj(rawContent)->v.OString.value;
    if (!PanWriteFile(filePathStr, contentStr)) {
        VmError(vm, RT_IME_STDFILE_WRITE_WRITE_FAIL, filePathStr);
        return MakeNil();
    }
    return MakeNil();
}

static PValue file_CreateFile(PVm *vm, PValue *args, u64 argc) {
    PValue rawFilePath = args[0];

    if (!IsValueObjType(rawFilePath, OT_STR)) {
        VmError(
            vm, RT_STDFILE_CREATEFILE_FILENAME_STR, ValueTypeToStr(rawFilePath)
        );
        return MakeNil();
    }

    char *filePathStr = ValueAsObj(rawFilePath)->v.OString.value;
    if (!PanCreateFile(filePathStr)) {
        VmError(vm, RT_IME_STDFILE_WRITE_WRITE_FAIL, filePathStr);
        return MakeNil();
    }
    return MakeNil();
}

static PValue file_CreateDir(PVm *vm, PValue *args, u64 argc) {
    PValue rawFilePath = args[0];

    if (!IsValueObjType(rawFilePath, OT_STR)) {
        VmError(vm, RT_STDFILE_CREATEDIR_FILENAME_STR);
        return MakeNil();
    }

    char *filePathStr = ValueAsObj(rawFilePath)->v.OString.value;
    if (!PanCreateDir(filePathStr)) {
        VmError(vm, RT_IME_STDFILE_CREATEDIR_CREATE_FAIL);
        return MakeNil();
    }
    return MakeNil();
}

#define FILE_STD_EXISTS      "বর্তমান"
#define FILE_STD_READ        "পড়ো"
#define FILE_STD_WRITE       "লেখো"
#define FILE_STD_CREATE_FILE "নতুন"
#define FILE_STD_CREATE_DIR  "নতুন_ফোল্ডার"

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
