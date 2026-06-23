# Copyright (c) 2022 Palash Bauri
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

set(PANKTI_SRC_FILES
  "${CMAKE_CURRENT_LIST_DIR}/argparse.c"
  "${CMAKE_CURRENT_LIST_DIR}/ast.c"
  "${CMAKE_CURRENT_LIST_DIR}/bengali.c"
  "${CMAKE_CURRENT_LIST_DIR}/builtins.c"
  "${CMAKE_CURRENT_LIST_DIR}/core.c"
  "${CMAKE_CURRENT_LIST_DIR}/diagonctx.c"
  "${CMAKE_CURRENT_LIST_DIR}/flags.c"
  "${CMAKE_CURRENT_LIST_DIR}/lexer.c"
  "${CMAKE_CURRENT_LIST_DIR}/parser.c"
  "${CMAKE_CURRENT_LIST_DIR}/printer.c"
  "${CMAKE_CURRENT_LIST_DIR}/strescape.c"
  "${CMAKE_CURRENT_LIST_DIR}/strpool.c"
  "${CMAKE_CURRENT_LIST_DIR}/terminal.c"
  "${CMAKE_CURRENT_LIST_DIR}/token.c"
  "${CMAKE_CURRENT_LIST_DIR}/unicode.c"
  "${CMAKE_CURRENT_LIST_DIR}/ustring.c"


  # Compiler Stuff

  "${CMAKE_CURRENT_LIST_DIR}/opcode.c"
  "${CMAKE_CURRENT_LIST_DIR}/compiler.c"
  "${CMAKE_CURRENT_LIST_DIR}/symtable.c"
  "${CMAKE_CURRENT_LIST_DIR}/vm.c"

  # Object

  "${CMAKE_CURRENT_LIST_DIR}/object.c"
  "${CMAKE_CURRENT_LIST_DIR}/arrayobj.c"
  "${CMAKE_CURRENT_LIST_DIR}/mapobj.c"
  "${CMAKE_CURRENT_LIST_DIR}/printobj.c"
  "${CMAKE_CURRENT_LIST_DIR}/tostrobj.c"

  # Utilities

  "${CMAKE_CURRENT_LIST_DIR}/utilfiles.c"
  "${CMAKE_CURRENT_LIST_DIR}/utilnums.c"
  "${CMAKE_CURRENT_LIST_DIR}/utilstr.c"

  # Garbage Collector

  "${CMAKE_CURRENT_LIST_DIR}/gc.c"
  "${CMAKE_CURRENT_LIST_DIR}/gcexpr.c"
  "${CMAKE_CURRENT_LIST_DIR}/gcobject.c"
  "${CMAKE_CURRENT_LIST_DIR}/gcstmt.c"

  # Stdlib
  "${CMAKE_CURRENT_LIST_DIR}/pstdlib.c"
  "${CMAKE_CURRENT_LIST_DIR}/stdmap.c"
  "${CMAKE_CURRENT_LIST_DIR}/stdarray.c"
  "${CMAKE_CURRENT_LIST_DIR}/stdmath.c"
  "${CMAKE_CURRENT_LIST_DIR}/stdsystem.c"
  "${CMAKE_CURRENT_LIST_DIR}/stdstring.c"
  "${CMAKE_CURRENT_LIST_DIR}/stdfile.c"

  # External
  "${CMAKE_CURRENT_LIST_DIR}/external/stb/stb_ds_impl.c"
  "${CMAKE_CURRENT_LIST_DIR}/external/gb/gb_string_impl.c"
  "${CMAKE_CURRENT_LIST_DIR}/external/xxhash/xxhash.c"

  # Generated Stuff
  "${CMAKE_CURRENT_LIST_DIR}/gen/diagon.c"
)

set(PANKTI_HEADER_FILES
  "${CMAKE_CURRENT_LIST_DIR}/alloc.h"
  "${CMAKE_CURRENT_LIST_DIR}/bengali.h"
  "${CMAKE_CURRENT_LIST_DIR}/core.h"
  "${CMAKE_CURRENT_LIST_DIR}/defaults.h"
  "${CMAKE_CURRENT_LIST_DIR}/gc.h"
  "${CMAKE_CURRENT_LIST_DIR}/keywords.h"
  "${CMAKE_CURRENT_LIST_DIR}/lexer.h"
  "${CMAKE_CURRENT_LIST_DIR}/object.h"
  "${CMAKE_CURRENT_LIST_DIR}/parser.h"
  "${CMAKE_CURRENT_LIST_DIR}/pstdlib.h"
  "${CMAKE_CURRENT_LIST_DIR}/strescape.h"
  "${CMAKE_CURRENT_LIST_DIR}/strpool.h"
  "${CMAKE_CURRENT_LIST_DIR}/terminal.h"
  "${CMAKE_CURRENT_LIST_DIR}/token.h"
  "${CMAKE_CURRENT_LIST_DIR}/ustring.h"
  "${CMAKE_CURRENT_LIST_DIR}/utils.h"

  # External
  "${CMAKE_CURRENT_LIST_DIR}/external/stb/stb_ds.h"
)

set(PANKTI_GFX_SRC_FILES
  "${CMAKE_CURRENT_LIST_DIR}/stdgfx.c"
  "${CMAKE_CURRENT_LIST_DIR}/gfxcore.c"
  "${CMAKE_CURRENT_LIST_DIR}/gfxdraw.c"
  "${CMAKE_CURRENT_LIST_DIR}/gfxfont.c"
  "${CMAKE_CURRENT_LIST_DIR}/gfxhelper.c"
  "${CMAKE_CURRENT_LIST_DIR}/gfxkeyboard.c"
)

set(PANKTI_MAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/main.c")
