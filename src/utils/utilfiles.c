#include "../include/alloc.h"
#include "../include/ptypes.h"
#include "../include/utils.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <uchar.h>

#if defined(PANKTI_OS_WIN)
#include <direct.h>
#include <io.h>
#define PAN_ACCESS(f)    _access(f, 0)
#define PAN_STAT_STRUCT  struct _stat
#define PAN_STAT(f, buf) _stat(f, buf)
#ifndef S_ISDIR
#define S_ISDIR(mode) (((mode) & _S_IFDIR) == _S_IFDIR)
#endif
#ifndef S_ISREG
#define S_ISREG(mode) (((mode) & _S_IFREG) == _S_IFREG)
#endif
#define PAN_MKDIR(f, mode) _mkdir(f)
#else
#include <unistd.h>
#define PAN_ACCESS(f)      access(f, F_OK)
#define PAN_STAT_STRUCT    struct stat
#define PAN_STAT(f, buf)   stat(f, buf)
#define PAN_MKDIR(f, mode) mkdir(f, mode)
#endif

// Potential bug: handling of non-seekable file
char *PanReadFile(const char *path) {
    char *text = NULL;

    if (path != NULL) {
        FILE *file = fopen(path, "rt");
        if (file != NULL) {
            fseek(file, 0, SEEK_END);
            long size = ftell(file);
            fseek(file, 0, SEEK_SET);

            if (size >= 0) {
                text = (char *)PCalloc(size + 1, sizeof(char));
                if (text == NULL) {
                    fclose(file);
                    return NULL;
                }

                fread(text, (size_t)size, 1, file);
                text[size] = '\0';

                // Check and Remove BOM from file
                if (size >= 3 && (uchar)text[0] == UTF8_BOM_0 &&
                    (uchar)text[1] == UTF8_BOM_1 &&
                    (uchar)text[2] == UTF8_BOM_2) {
                    memmove(text, text + 3, size - 3);
                    text[size - 3] = '\0';
                }
            }

            fclose(file);
        }
    }

    return text;
}

bool PanWriteFile(const char *filepath, const char *str) {
    if (filepath == NULL || str == NULL) {
        return false;
    }

    FILE *file = fopen(filepath, "wt");
    if (file == NULL) {
        return false;
    }

    fprintf(file, "%s", str); // todo: error check
    fclose(file);

    return true;
}

bool PanCreateFile(const char *filepath) {
    if (filepath == NULL) {
        return false;
    }

    FILE *file = fopen(filepath, "w");
    if (file == NULL) {
        return false;
    }
    fclose(file);

    return true;
}

bool PanCreateDir(const char *filepath) {
    if (filepath == NULL) {
        return false;
    }
    if (PAN_MKDIR(filepath, 0777) == -1) {
        return false;
    }

    return true;
}

bool DoesFileExists(const char *filepath) {
    if (filepath == NULL) {
        return false;
    }
    if (PAN_ACCESS(filepath) != -1) {
        return true;
    }

    return false;
}

bool PanIsPathDir(const char *filepath) {
    if (filepath == NULL) {
        return false;
    }

    PAN_STAT_STRUCT buf = {0};
    PAN_STAT(filepath, &buf);
    return S_ISDIR(buf.st_mode);
}

bool PanIsPathFile(const char *filepath) {
    if (filepath == NULL) {
        return false;
    }
    PAN_STAT_STRUCT buf = {0};
    if (PAN_STAT(filepath, &buf) != 0) {
        return false;
    }

    return S_ISREG(buf.st_mode);
}
