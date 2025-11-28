#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uchar.h>

#include "include/ptypes.h"
#include "include/ustring.h"
#include "include/alloc.h"


static u32 uiterGetACp(const char *str, size_t len, size_t *ate) {
    if (len == 0) {
        *ate = 0;
        return 0;
    }

    unsigned char c1 = (unsigned char)str[0];

    // 1 byte ASCII
    // 0x80 => 1 0 0 0 0 0 0 0
    if ((c1 & 0x80) == 0) {
        *ate = 1;
        return c1;
    }

    // 2 byte char
    if ((c1 & 0xE0) == 0xC0) {
        if (len < 2) {
            *ate = 1;
            return UITER_INVALID_CODEPOINT;
        }

        if ((str[1] & 0xC0) != 0x80) {
            *ate = 1;
            return UITER_INVALID_CODEPOINT;
        }

        *ate = 2;
        return (u32)((c1 & 0x1F) << 6) | (u32)(str[1] & 0x3F);
    }

    // 3 byte char
    if ((c1 & 0xF0) == 0xE0) {
        if (len < 3) {
            *ate = 1;
            return UITER_INVALID_CODEPOINT;
        }
        if (((str[1] & 0xC0) != 0x80) || ((str[2] & 0xC0) != 0x80)) {
            *ate = 1;
            return UITER_INVALID_CODEPOINT;
        }

        *ate = 3;
        return (u32)((c1 & 0x0F) << 12) |
               (u32)((str[1] & 0x3F) << 6) | (u32)(str[2] & 0x3F);
    }

    if ((c1 & 0xF8) == 0xF0) {
        if (len < 4) {
            *ate = 1;
            return UITER_INVALID_CODEPOINT;
        }

        if ((str[1] & 0xC0) != 0x80 || (str[2] & 0xC0) != 0x80 ||
            (str[3] & 0xC0) != 0x80) {
            *ate = 1;
            return UITER_INVALID_CODEPOINT;
        }

        *ate = 4;
        return (
            (u32)((c1 & 0x07) << 18) | (u32)((str[1] & 0x3F) << 12) |
            (u32)((str[2] & 0x3F) << 6) | (u32)(str[3] & 0x3F)
        );
    }

    *ate = 1;
    return UITER_INVALID_CODEPOINT;
}

static void uiterFillPeekBuffer(UIter *it) {
    it->peekCount = 0;
    size_t curpos = it->pos;
    for (int i = 0; i < UITER_PEEK_BUFFER_SIZE; i++) {
        if (curpos >= it->len) {
            break;
        }
        size_t ate = 0;
        u32 cp = uiterGetACp(it->str + curpos, it->len - curpos, &ate);
        if (ate > 0) {
            it->peekBuf[i] = cp;
            it->peekCount++;
            curpos += ate;
        } else {
            break;
        }
    }
}

UIter *NewUIterator(const char *text) {
    if (text == NULL) {
        return NULL;
    }

    UIter *it = (UIter *)PMalloc(sizeof(UIter));
    if (it == NULL) {
        return NULL;
    }
    it->str = text;
    it->len = (size_t)strlen(text);
    it->pos = 0;
    it->peekCount = 0;
    uiterFillPeekBuffer(it);
    return it;
}
void FreeUIterator(UIter *iter) {
    if (iter == NULL) {
        return;
    }
    PFree(iter);
}
bool UIterIsEnd(const UIter *iter) { return iter->peekCount == 0; }
u32 UIterPeek(const UIter *iter, int offset) {
    if (offset < 0 || offset >= iter->peekCount) {
        return 0;
    }

    return iter->peekBuf[offset];
}
void UIterAdvance(UIter *iter) {
    if (iter->peekCount == 0) {
        return;
    }

    // how much first item in peek buffer ate?
    size_t ate = 0;
    uiterGetACp(iter->str + iter->pos, iter->len - iter->pos, &ate);
    if (ate > 0) {
        iter->pos += ate;
    } else {
        iter->pos += 1;
    }

    uiterFillPeekBuffer(iter);
}
u32 UIterNext(UIter *it) {
    if (UIterIsEnd(it)) {
        return 0;
    }

    u32 curCp = it->peekBuf[0];
    UIterAdvance(it);
    return curCp;
}

void U32ToU8(u32 value, u8 result[4]) {
    result[0] = (u8)(value & 0xFF);
    result[1] = (u8)((value >> 8) & 0xFF);
    result[2] = (u8)((value >> 16) & 0xFF);
    result[3] = (u8)((value >> 24) & 0xFF);
}

void DebugPeekBuffer(const UIter *it) {
    printf("PBuf[");
    for (int i = 0; i < it->peekCount; i++) {
        u32 pk = it->peekBuf[i];
        u8 x[4];
        U32ToU8(pk, x);
        printf("|%d:U+%04X:'%s'| ", i, pk, x);
    }
    printf("]\n");
}
