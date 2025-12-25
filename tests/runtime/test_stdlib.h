#ifndef RUNTIME_TEST_STDLIB_H
#define RUNTIME_TEST_STDLIB_H

#include "tester.h"
#include "../include/utest.h"
#ifdef __cplusplus
extern "C" {
#endif

UTEST(RuntimeTest, StdMath){ GoldenTest("stdmath"); }
UTEST(RuntimeTest, StdMap){ GoldenTest("stdmap"); }
UTEST(RuntimeTest, StdArray){ GoldenTest("stdarray"); }



#ifdef __cplusplus
}
#endif

#endif
