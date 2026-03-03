#ifndef RUNTIME_TEST_IF_H
#define RUNTIME_TEST_IF_H

#include "tester.h"
#include "../include/utest.h"
#ifdef __cplusplus
extern "C" {
#endif

UTEST(RuntimeTest, IfElse){ GoldenTest("if_else"); }
UTEST(RuntimeTest, IfElseNested){ GoldenTest("if_else_nested"); }



#ifdef __cplusplus
}
#endif

#endif
