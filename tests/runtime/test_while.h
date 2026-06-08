/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

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
UTEST(RuntimeTest, WhileNested){ GoldenTest("while_nested"); }
UTEST(RuntimeTest, WhileNestedBreak){ GoldenTest("while_nested_break"); }
UTEST(RuntimeTest, WhileNestedContinue){ GoldenTest("while_nested_continue"); }
//UTEST(RuntimeTest, *){ GoldenTest("*"); }



#ifdef __cplusplus
}
#endif

#endif
