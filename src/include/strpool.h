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

typedef struct PStringPool {
    SPoolSet table;
    u64 count;
} PStringPool;

PStringPool *NewStringPool(void);
void FreeStringPool(PStringPool *sp);

PObj *StringPoolFind(PStringPool *sp, const char *val, u64 hash);
bool StringPoolInsert(PStringPool *sp, PObj *strObj);
bool StringPoolRemove(PStringPool *sp, PObj *strObj);
void StringPoolMark(PStringPool *sp);

#ifdef __cplusplus
}
#endif

#endif
