#ifndef BENGALI_H
#define BENGALI_H

#include <stdint.h>
#include <stdbool.h>
#include <stdint.h>

#define BN_NUM_0 0x09E6
#define BN_NUM_1 0x09E7
#define BN_NUM_2 0x09E8
#define BN_NUM_3 0x09E9
#define BN_NUM_4 0x09EA
#define BN_NUM_5 0x09EB
#define BN_NUM_6 0x09EC
#define BN_NUM_7 0x09ED
#define BN_NUM_8 0x09EE
#define BN_NUM_9 0x09EF

#define BN_RANGE_START 0x0980
#define BN_RANGE_END   0x09FE

bool IsBnNumber(uint32_t c);
bool IsBnChar(uint32_t c);
uint8_t GetEnFromBnNum(uint32_t c);


#endif
