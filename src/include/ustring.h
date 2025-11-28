#ifndef USTRING_H
#define USTRING_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "ptypes.h"

// How many codepoints the Iterator can peek
#define UITER_PEEK_BUFFER_SIZE 3
// Placeholder return of invalid codepoints
#define UITER_INVALID_CODEPOINT 0xFFFD

// Unicode Iterator Object
typedef struct UIter {
    // Raw UTF-8 encoded string. Reference to source from Lexer object
    const char *str;
    // How many characters is in `str`.
    size_t len;
    // Current position of iterator
    size_t pos;

    // Stack allocated peek buffer.
    // Index 0 is next codepoint.
    u32 peekBuf[UITER_PEEK_BUFFER_SIZE];

    // How many peekable codepoint is available
    int peekCount;
} UIter;

// New Unicode string iterator object
// `text` = UTF-8 encoded string
UIter *NewUIterator(const char *text);

// Free Iterator object
void FreeUIterator(UIter *iter);

// Is the iterator at the end of string
bool UIterIsEnd(const UIter *iter);

// Peek the next codepoint
// `offset` = returns `current codepoint index + offset`
u32 UIterPeek(const UIter *iter, int offset);

// Advance the iterator
void UIterAdvance(UIter *iter);

// Advance the iterator and return the just passed codepoint
u32 UIterNext(UIter *it);

// Convert the UTF-32 encoded codepoint to UTF-8 encoded character array
// `value` = UTF-32 encoded codepoint
// `result` = buffer to write the UTF-8 encoded bytes
void U32ToU8(u32 value, u8 result[4]);

// Debug/Print the peek buffer
void DebugPeekBuffer(const UIter *it);

#ifdef __cplusplus
}
#endif

#endif
