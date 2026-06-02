#ifndef RUNTIME_TEST_BASICS_H
#define RUNTIME_TEST_BASICS_H

#include "tester.h"
#include "../include/utest.h"
#ifdef __cplusplus
extern "C" {
#endif

UTEST(RuntimeTest, BasicArithmetic){ GoldenTest("arithmetic"); }
UTEST(RuntimeTest, Function){ GoldenTest("func_basic"); }
UTEST(RuntimeTest, LogicalOps){ GoldenTest("logical_ops"); }
UTEST(RuntimeTest, NestedData){ GoldenTest("nested_data"); }
UTEST(RuntimeTest, SubscriptAssign){ GoldenTest("subscript_assign"); }
UTEST(RuntimeTest, MixedSyntax){ GoldenTest("mixed_syntax"); }
UTEST(RuntimeTest, Truthiness){ GoldenTest("truthiness"); }
UTEST(RuntimeTest, FuncFirstClass){ GoldenTest("func_firstclass"); }


#ifdef __cplusplus
}
#endif

#endif
