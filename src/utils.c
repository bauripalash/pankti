#include "include/utils.h"
#include "external/xxhash/xxhash.h"
#include "include/alloc.h"
#include "include/bengali.h"
#include "include/ustring.h"

#include <ctype.h>
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

            if (size >= 0) {
                text = (char *)PCalloc((size_t)size + 1, sizeof(char));
                if (text == NULL) {
					fclose(file);
                    return NULL;
                }

                fread(text, (size_t)size, 1, file);
                text[size] = '\0';
            }

            fclose(file);

        }
    }

    return text;
}

char *SubString(const char *str, pusize start, pusize end) {
    if (str == NULL) {
        return NULL;
    }

    pusize len = (pusize)strlen(str);
    if (start > end || end > len) {
        return NULL;
    }

    pusize sublen = end - start;
    char *result = PMalloc(((size_t)sublen + 1) * sizeof(char));
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

char *StrDuplicate(const char *str, pusize len) {
    if (str == NULL) {
        return NULL;
    }

    char *dup = PCalloc((size_t)len + 1, sizeof(char));
    if (dup == NULL) {
        return NULL;
    }
    strncpy(dup, str, len);
    return dup;
}

pusize StrLength(const char *str) { return (pusize)strlen(str); }

bool MatchKW(const char *s, const char *en, const char *pn, const char *bn) {
    return StrEqual(s, en) || StrEqual(s, pn) || StrEqual(s, bn);
}

pchar32 U8ToU32(const unsigned char *str) {
    return ((pchar32)(str[0] & 0x0F) << 12) | ((pchar32)(str[1] & 0x3F) << 6) | (pchar32)(str[2] & 0x3F);
}

char *BoolToString(bool v) {
    if (v) {
        return "true";
    }
    return "false";
}

uint64_t StrHash(const char *str, pusize len, uint64_t seed) {
    XXH64_hash_t hash = XXH64(str, len, (XXH64_hash_t)seed);
    return (uint64_t)hash;
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

char *StrJoin(
    const char *a, pusize alen, const char *b, pusize blen, bool *ok
) {
    pusize outlen = alen + blen + 1;
    char *output = PCalloc((size_t)outlen, sizeof(char));
    if (output == NULL) {
        *ok = false;
        return NULL;
    }

    strncat(output, a, alen);
    strncat(output, b, blen);
    output[outlen] = '\0';
    *ok = true;
    return output;
}

double NumberFromStr(const char *lexeme, pusize len, bool *ok) {

    char *buf = PCalloc((size_t)(len + 1), sizeof(char));
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

unsigned char ToHex2Bytes(char c1, char c2) {
    unsigned char nb1, nb2;

    if (isalpha(c1)) {
        nb1 = (unsigned char)(tolower(c1) - 'a') + 10;
    } else if (isdigit(c1)) {
        nb1 = (unsigned char)(c1 - '0');
    } else {
        return 0;
    }

    if (isalpha(c2)) {
        nb2 = (unsigned char)(tolower(c2) - 'a') + 10;
    } else if (isdigit(c2)) {
        nb2 = (unsigned char)(c2 - '0');
    } else {
        return 0;
    }

    return (unsigned char)(nb1 << 4) | nb2;
}

unsigned char HexStrToByte(char *str, int len) {
    if (len == 2) {
        return ToHex2Bytes(str[0], str[1]);
    }

    return 0;
}
