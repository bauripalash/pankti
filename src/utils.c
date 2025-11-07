#include "include/utils.h"
#include "include/alloc.h"
#include "include/bengali.h"
#include "include/ustring.h"
#include <math.h>
#include <stdarg.h>
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

// Source:
// https://github.com/raysan5/raylib/blob/dfc94f64d1e1db5231a68e8ea968378df16f2292/src/rtext.c#L1504
const char *StrFormat(const char *text, ...) {
    static char buffers[MAX_STRFORMAT_BUF][MAX_STRBUF_LENGTH] = {0};
    static int index = 0;

    char *curbuf = buffers[index];
    memset(curbuf, 0, MAX_STRBUF_LENGTH);
    va_list args;
    va_start(args, text);
    int reqLen = vsnprintf(curbuf, MAX_STRBUF_LENGTH, text, args);
    va_end(args);

    if (reqLen >= MAX_STRBUF_LENGTH) {
        char *trunc = buffers[index] + MAX_STRBUF_LENGTH - 4;
        sprintf(trunc, "...");
    }

    index += 1;
    if (index >= MAX_STRFORMAT_BUF) {
        index = 0;
    }

    return curbuf;
}

// Source:
// https://github.com/raysan5/raylib/blob/dfc94f64d1e1db5231a68e8ea968378df16f2292/src/rtext.c#L1836
char **StrSplit(const char *text, char delimiter, int *count) {
    static char *buffers[MAX_STRSPLIT_COUNT] = {
        NULL
    }; // Pointers to buffer[] text data
    static char buffer[MAX_STRBUF_LENGTH] = {
        0
    }; // Text data with '\0' separators
    memset(buffer, 0, MAX_STRBUF_LENGTH);

    buffers[0] = buffer;
    int counter = 0;
    if (text != NULL) {
        counter = 1;

        for (int i = 0; i < MAX_STRBUF_LENGTH; i++) {
            buffer[i] = text[i];
            if (buffer[i] == '\0') {
                break;
            } else if (buffer[i] == delimiter) {
                buffer[i] = '\0';
                buffers[counter] = buffer + i + 1;
                counter++;
                if (counter == MAX_STRSPLIT_COUNT) {
                    break;
                }
            }
        }
    }

    *count = counter;
    return buffers;
}

double NumberFromStr(const char *lexeme, int len, bool *ok) {

    char *buf = PCalloc((len + 1), sizeof(char));
    char *ptr = buf;
    if (buf == NULL) {
        *ok = false;
        return -1;
    }

    UIter *iter = NewUIterator(lexeme);
    int index = 0;

    while (!UIterIsEnd(iter)) {
        char32_t ch = UIterNext(iter);
        if (ch == '.') {
            buf[index++] = '.';
            continue;
        }
        buf[index++] = GetEnFromBnNum(ch);
    }

    double value = atof(buf);
    *ok = true;
    FreeUIterator(iter);
    PFree(buf);
    return value;
}

bool IsDoubleInt(double d) { return (floor(d) == ceil(d)); }
