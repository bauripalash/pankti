#ifndef RUNTIME_TEST_CLOSURE_H
#define RUNTIME_TEST_CLOSURE_H

#include "tester.h"
#include "../include/utest.h"
#ifdef __cplusplus
extern "C" {
#endif

UTEST(RuntimeTest, Nested_Closure){ GoldenTest("nested_closure"); }



#ifdef __cplusplus
}
#endif

#endif
