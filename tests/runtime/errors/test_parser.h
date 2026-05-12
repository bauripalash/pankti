#ifndef RUNTIME_TEST_ERR_PARSER_H
#define RUNTIME_TEST_ERR_PARSER_H

#include "../tester.h"
#include "../../include/utest.h"
#ifdef __cplusplus
extern "C" {
#endif

UTEST(RuntimeErrorTest, LetMissingIdent){ ErrorTest("let_missing_ident"); }


#ifdef __cplusplus
}
#endif

#endif
