#ifndef RUNTIME_TEST_WHILE_H
#define RUNTIME_TEST_WHILE_H

#include "tester.h"
#include "../include/utest.h"
#ifdef __cplusplus
extern "C" {
#endif

UTEST(RuntimeTest, SimpleWhile){ GoldenTest("while_loop"); }
UTEST(RuntimeTest, WhileBreak){ GoldenTest("while_break"); }
UTEST(RuntimeTest, WhileContinue){ GoldenTest("while_continue"); }
UTEST(RuntimeTest, WhileFizzBuzz){ GoldenTest("while_fizzbuzz"); }
//UTEST(RuntimeTest, *){ GoldenTest("*"); }



#ifdef __cplusplus
}
#endif

#endif
