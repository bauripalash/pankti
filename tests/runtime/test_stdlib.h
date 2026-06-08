/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef RUNTIME_TEST_STDLIB_H
#define RUNTIME_TEST_STDLIB_H

#include "tester.h"
#include "../include/utest.h"
#ifdef __cplusplus
extern "C" {
#endif

UTEST(RuntimeTest, StdString){ GoldenTest("stdstring"); }
UTEST(RuntimeTest, StdMath){ GoldenTest("stdmath"); }
UTEST(RuntimeTest, StdMap){ GoldenTest("stdmap"); }
UTEST(RuntimeTest, StdArray){ GoldenTest("stdarray"); }
UTEST(RuntimeTest, StdFile){ GoldenTest("stdfile"); }
UTEST(RuntimeTest, StdGraphics){ GoldenTest("stdgraphics"); }
UTEST(RuntimeTest, StdSystem){ GoldenTest("stdsystem"); }



#ifdef __cplusplus
}
#endif

#endif
