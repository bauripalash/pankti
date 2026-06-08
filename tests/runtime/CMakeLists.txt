# Copyright (c) 2022 Palash Bauri
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

set(IS_GFX_BUILD FALSE)
set(RUNTIME_TEST_SRC test_runtime.c)
set(RUNTIME_TEST_EXE pankti_runtime_tests)

add_executable(${RUNTIME_TEST_EXE}
    ${RUNTIME_TEST_SRC}
)

if (NOT IS_MSVC)
    target_link_libraries(${RUNTIME_TEST_EXE} PRIVATE m)
endif()
