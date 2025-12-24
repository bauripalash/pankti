#ifndef UNICODE_H
#define UNICODE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ptypes.h"

// Grapheme Parsing Error
typedef enum GraphemeError {
    // No Grapheme Error
    GR_ERR_OK,
    // Grapheme Index is out of range
    GR_ERR_INDEX_OUT_RANGE,
    // No grapheme found in string
    GR_ERR_EMPTY,
    // Memory Error When Creating a string
    GR_ERR_MEM,
} GraphemeError;

u64 GetGraphemeCount(const char *str, u64 len);
char *GetGraphemeAt(const char *str, u64 len, u64 index, GraphemeError *err);
char **GetGraphemeArray(const char *str, u64 len, GraphemeError *err);

#ifdef __cplusplus
}
#endif

#endif
