#include "../include/alloc.h"
#include "../include/ptypes.h"
#include "../include/utils.h"
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uchar.h>

#if defined(PANKTI_OS_WIN)
#include <direct.h>
#include <io.h>
#define PAN_ACCESS(f) _access(f, 0)
#else
#include <unistd.h>
#define PAN_ACCESS(f) access(f, F_OK)
#endif

// Potential bug: handling of non-seekable file
char *ReadFile(const char *path) {
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

bool DoesFileExists(const char *filepath) {
    if (filepath == NULL) {
        return NULL;
    }
    if (PAN_ACCESS(filepath) != -1) {
        return true;
    }

    return false;
}
