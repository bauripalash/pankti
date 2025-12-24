#include "include/unicode.h"
#include "external/libgrapheme/grapheme.h"
#include "external/stb/stb_ds.h"
#include "include/alloc.h"
#include "include/ptypes.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

u64 GetGraphemeCount(const char *str, u64 len) {
    if (str == NULL || len == 0) {
        return 0;
    }
    u64 count = 0;
    size_t offset = 0;

    while (offset < len) {
        size_t glen =
            grapheme_next_character_break_utf8(str + offset, len - offset);
        if (glen == 0) {
            break;
        }

        count++;
        offset += glen;
    }

    return count;
}
char *GetGraphemeAt(const char *str, u64 len, u64 index, GraphemeError *err) {
    if (str == NULL || len == 0) {
        *err = GR_ERR_EMPTY;
        return NULL;
    }

    u64 count = GetGraphemeCount(str, len);
    if (count == 0) {
        *err = GR_ERR_EMPTY;
        return NULL;
    }
    if (index >= count) {
        *err = GR_ERR_INDEX_OUT_RANGE;
        return NULL;
    }
    count = 0;
    size_t offset = 0;

    while (offset < len) {
        size_t glen =
            grapheme_next_character_break_utf8(str + offset, len - offset);
        if (glen == 0) {
            break;
        }

        if (count == index) {
            char *temp = PMalloc(sizeof(char) * (glen + 1));
            if (temp == NULL) {
                *err = GR_ERR_MEM;
                return NULL;
            }

            strncpy(temp, str + offset, glen);
            temp[glen] = '\0';
            *err = GR_ERR_OK;
            return temp;
        }

        count++;
        offset += glen;
    }
    *err = GR_ERR_EMPTY;
    return NULL;
}
char **GetGraphemeArray(const char *str, u64 len, GraphemeError *err) {
    if (str == NULL || len == 0) {
        *err = GR_ERR_EMPTY;
        return NULL;
    }

    u64 count = GetGraphemeCount(str, len);
    if (count == 0) {
        *err = GR_ERR_EMPTY;
        return NULL;
    }

    char **result = NULL;
    arrsetcap(result, count);
    size_t offset = 0;

    while (offset < len) {
        size_t glen =
            grapheme_next_character_break_utf8(str + offset, len - offset);
        if (glen == 0) {
            break;
        }

        char *temp = PMalloc(sizeof(char) + (glen + 1));
        strncpy(temp, str + offset, glen);
        temp[glen] = '\0';
        arrput(result, temp);
        offset += glen;
    }

    if (result == NULL) {
        *err = GR_ERR_EMPTY;
        return NULL;
    }

    *err = GR_ERR_OK;
    return result;
}
