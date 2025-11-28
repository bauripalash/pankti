#ifndef BENGALI_H
#define BENGALI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "ptypes.h"

// UTF32 Bengali Number `0`
#define BN_NUM_0 0x09E6

// UTF32 Bengali Number `1`
#define BN_NUM_1 0x09E7

// UTF32 Bengali Number `2`
#define BN_NUM_2 0x09E8

// UTF32 Bengali Number `3`
#define BN_NUM_3 0x09E9

// UTF32 Bengali Number `4`
#define BN_NUM_4 0x09EA

// UTF32 Bengali Number `5`
#define BN_NUM_5 0x09EB

// UTF32 Bengali Number `6`
#define BN_NUM_6 0x09EC

// UTF32 Bengali Number `7`
#define BN_NUM_7 0x09ED

// UTF32 Bengali Number `8`
#define BN_NUM_8 0x09EE

// UTF32 Bengali Number `9`
#define BN_NUM_9 0x09EF

// Bengali Unicode range Start
#define BN_RANGE_START 0x0980
// Bengali Unicode range End
#define BN_RANGE_END 0x09FE

// Check if UTF32 encoded char is a Bengali Number
bool IsBnNumber(u32 c);
// Check if UTF32 encoded char is in Bengali Unicode Range
bool IsBnChar(u32 c);

// Convert UTF32 encoded number to english number;
// Returns => char with number value, return as is if the `c` is English number
char GetEnFromBnNum(u32 c);

#ifdef __cplusplus
}
#endif
#endif
