#ifndef PANKTI_STR_TABLE_H
#define PANKTI_STR_TABLE_H

#ifdef __cplusplus
extern "C" {
#endif
#include "object.h"
#include "ptypes.h"
#include <stdbool.h>

// NOLINTBEGIN(clang-analyzer-*)
#define NAME   SPoolSet
#define KEY_TY PObj *
#define HEADER_MODE
#include "../external/verstable/verstable.h"
// NOLINTEND(clang-analyzer-*)

// String Interning Pool
typedef struct PStringPool {
    // String pool set
    SPoolSet table;
    u64 count;
} PStringPool;

// Create new String Interning Pool
PStringPool *NewStringPool(void);
// Free the String Interning Pool
void FreeStringPool(PStringPool *sp);

// Look for a string object with value of val
PObj *StringPoolFind(PStringPool *sp, const char *val, u64 hash);

// Insert new String object to pool
bool StringPoolInsert(PStringPool *sp, PObj *strObj);
// Remove String from pool
bool StringPoolRemove(PStringPool *sp, PObj *strObj);
// Remove Unmarked String objects from pool
void StringPoolRemoveUnmarked(PStringPool *sp);

#ifdef __cplusplus
}
#endif

#endif
