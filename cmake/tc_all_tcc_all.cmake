# Copyright (c) 2022 Palash Bauri
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

# Specify the TCC compiler
set(CMAKE_C_COMPILER tcc)

# set(CMAKE_C_COMPILER_WORKS 1) 
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

message(STATUS "TinyCC Compiler doesn't support kb_text_shape.h thus stdgfx is not supported")
