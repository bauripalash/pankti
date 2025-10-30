#include "include/utils.h"
#include "include/alloc.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uchar.h>

char *ReadFile(const char *path) {
    char *text = NULL;

    if (path != NULL) {
        FILE *file = fopen(path, "rt");
        if (file != NULL) {
            fseek(file, 0, SEEK_END);
            long size = ftell(file);
            fseek(file, 0, SEEK_SET);

            if (size > 0) {
                text = (char *)PCalloc(size + 1, sizeof(char));
                if (text == NULL) {
                    return NULL;
                }

                fread(text, size, 1, file);
                fclose(file);
                text[size] = '\0';
            }
        }
    }

    return text;
}

char *SubString(const char *str, int start, int end) {
    if (str == NULL) {
        return NULL;
    }

    size_t len = strlen(str);
    if (start > end || end > len) {
        return NULL;
    }

    size_t sublen = end - start;
    char *result = PMalloc((sublen + 1) * sizeof(char));
    if (result == NULL) {
        return NULL;
    }
    memcpy(result, str + start, sublen);
    result[sublen] = '\0';
    return result;
}

bool StrEqual(const char *str1, const char *str2) {
    return strcmp(str1, str2) == 0;
}

long StrLength(const char *str) { return strlen(str); }

bool MatchKW(const char *s, const char *en, const char *pn, const char *bn) {
    return StrEqual(s, en) || StrEqual(s, pn) || StrEqual(s, bn);
}

char32_t U8ToU32(const unsigned char *str) {
    return ((str[0] & 0x0F) << 12) | ((str[1] & 0x3F) << 6) | (str[2] & 0x3F);
}

char *BoolToString(bool v) {
    if (v) {
        return "true";
    }
    return "false";
}

uint32_t Fnv1a(const char *str, int len) {
    uint32_t hash = 2166136261u;
    for (int i = 0; i < len; i++) {
        hash ^= (uint8_t)str[i];
        hash *= 16777619;
    }
    return hash;
}
