/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef RUNTIME_TEST_INTERNAL_H
#define RUNTIME_TEST_INTERNAL_H

#include "tester.h"
#include "../include/utest.h"
#ifdef __cplusplus
extern "C" {
#endif

UTEST(RuntimeTest, Internal_Types){ GoldenTest("types"); }
UTEST(RuntimeTest, Internal_ScriptWithBOM){ GoldenTest("arithmetic_bom"); }
UTEST(RuntimeTest, Internal_NilBehavior){ GoldenTest("nil_behavior"); }
UTEST(RuntimeTest, Internal_ValuePrint){ GoldenTest("valueprint"); }
UTEST(RuntimeTest, Internal_Builtins){ GoldenTest("builtins"); }

#ifdef __cplusplus
}
#endif

#endif
