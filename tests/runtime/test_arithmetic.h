#ifndef RUNTIME_TEST_ARITHMETIC_H
#define RUNTIME_TEST_ARITHMETIC_H

#include "tester.h"
#include "../include/utest.h"
#ifdef __cplusplus
extern "C" {
#endif

UTEST(RuntimeTest, BasicArithmetic){ GoldenTest("arithmetic"); }
UTEST(RuntimeTest, BasicArithmeticBOM){ GoldenTest("arithmetic_bom"); }



#ifdef __cplusplus
}
#endif

#endif
