#include "include/bengali.h"
#include <stdint.h>

bool IsBnNumber(uint32_t c) { return (c >= BN_NUM_0 && c <= BN_NUM_9); }
bool IsBnChar(uint32_t c) { return (c >= BN_RANGE_START && c <= BN_RANGE_END); }
uint8_t GetEnFromBnNum(uint32_t c) {
    switch (c) {
        case BN_NUM_0: return '0';
        case BN_NUM_1: return '1';
        case BN_NUM_2: return '2';
        case BN_NUM_3: return '3';
        case BN_NUM_4: return '4';
        case BN_NUM_5: return '5';
        case BN_NUM_6: return '6';
        case BN_NUM_7: return '7';
        case BN_NUM_8: return '8';
        case BN_NUM_9: return '9';
        default: return (uint8_t)c;
    }
}
