#ifndef USTRING_H
#define USTRING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define UITER_PEEK_BUFFER_SIZE 3
#define UITER_INVALID_CODEPOINT 0xFFFD

typedef struct UIter{
	const char * str;
	size_t len;
	size_t pos;

	uint32_t peekBuf[UITER_PEEK_BUFFER_SIZE];
	int peekCount;
}UIter;

UIter * NewUIterator(const char * text);
void FreeUIterator(UIter * iter);
bool UIterIsEnd(const UIter * iter);
uint32_t UIterPeek(const UIter * iter, int offset);
void UIterAdvance(UIter * iter);
uint32_t UIterNext(UIter * it);


void U32ToU8(uint32_t value, uint8_t result[4]);
void DebugPeekBuffer(const UIter * it);

#endif
