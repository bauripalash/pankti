#include "include/bengali.h"
#include <stdint.h>

bool IsBnNumber(uint32_t c){
	return c >= BN_NUM_0 && c <= BN_NUM_9;
}
bool IsBnChar(uint32_t c){
	return c >= BN_RANGE_START && c <= BN_RANGE_END;
}
uint8_t GetEnFromBnNum(uint32_t c){
	uint8_t result = 0;
	
	switch (c) {
		case BN_NUM_0: result = 0;break;
		case BN_NUM_1: result = 1;break;
		case BN_NUM_2: result = 2;break;
		case BN_NUM_3: result = 3;break;
		case BN_NUM_4: result = 4;break;
		case BN_NUM_5: result = 5;break;
		case BN_NUM_6: result = 6;break;
		case BN_NUM_7: result = 7;break;
		case BN_NUM_8: result = 8;break;
		case BN_NUM_9: result = 9;break;
		default:break;
	}

	return result;
}
