set(PANKTI_SRC_FILES
  "${CMAKE_CURRENT_LIST_DIR}/ast.c"
  "${CMAKE_CURRENT_LIST_DIR}/bengali.c"
  "${CMAKE_CURRENT_LIST_DIR}/core.c"
  "${CMAKE_CURRENT_LIST_DIR}/env.c"
  #"${CMAKE_CURRENT_LIST_DIR}/interpreter.c"
  "${CMAKE_CURRENT_LIST_DIR}/lexer.c"
  "${CMAKE_CURRENT_LIST_DIR}/native.c"
  "${CMAKE_CURRENT_LIST_DIR}/object.c"
  "${CMAKE_CURRENT_LIST_DIR}/parser.c"
  "${CMAKE_CURRENT_LIST_DIR}/printer.c"
  "${CMAKE_CURRENT_LIST_DIR}/strescape.c"
  "${CMAKE_CURRENT_LIST_DIR}/token.c"
  "${CMAKE_CURRENT_LIST_DIR}/unicode.c"
  "${CMAKE_CURRENT_LIST_DIR}/ustring.c"
  "${CMAKE_CURRENT_LIST_DIR}/utils.c"

  # Compiler Stuff

  "${CMAKE_CURRENT_LIST_DIR}/opcode.c"
  "${CMAKE_CURRENT_LIST_DIR}/compiler.c"
  "${CMAKE_CURRENT_LIST_DIR}/symtable.c"
  "${CMAKE_CURRENT_LIST_DIR}/vm.c"
  

  

  "${CMAKE_CURRENT_LIST_DIR}/gc/gc.c"
  "${CMAKE_CURRENT_LIST_DIR}/gc/gcexpr.c"
  "${CMAKE_CURRENT_LIST_DIR}/gc/gcobject.c"
  "${CMAKE_CURRENT_LIST_DIR}/gc/gcstmt.c"

  # Stdlib
  "${CMAKE_CURRENT_LIST_DIR}/stdlib/pstdlib.c"
  "${CMAKE_CURRENT_LIST_DIR}/stdlib/stdmap.c"
  "${CMAKE_CURRENT_LIST_DIR}/stdlib/stdarray.c"
  "${CMAKE_CURRENT_LIST_DIR}/stdlib/stdmath.c"
  "${CMAKE_CURRENT_LIST_DIR}/stdlib/stdos.c"
  "${CMAKE_CURRENT_LIST_DIR}/stdlib/stdstring.c"

  # External
  "${CMAKE_CURRENT_LIST_DIR}/external/stb/stb_ds_impl.c"
  "${CMAKE_CURRENT_LIST_DIR}/external/gb/gb_string_impl.c"
  "${CMAKE_CURRENT_LIST_DIR}/external/xxhash/xxhash.c"
)

set(PANKTI_HEADER_FILES
  "${CMAKE_CURRENT_LIST_DIR}/include/alloc.h"
  "${CMAKE_CURRENT_LIST_DIR}/include/ansicolors.h"
  "${CMAKE_CURRENT_LIST_DIR}/include/ast.h"
  "${CMAKE_CURRENT_LIST_DIR}/include/bengali.h"
  "${CMAKE_CURRENT_LIST_DIR}/include/core.h"
  "${CMAKE_CURRENT_LIST_DIR}/include/defaults.h"
  "${CMAKE_CURRENT_LIST_DIR}/include/env.h"
  "${CMAKE_CURRENT_LIST_DIR}/include/gc.h"
  #"${CMAKE_CURRENT_LIST_DIR}/include/interpreter.h"
  "${CMAKE_CURRENT_LIST_DIR}/include/keywords.h"
  "${CMAKE_CURRENT_LIST_DIR}/include/lexer.h"
  "${CMAKE_CURRENT_LIST_DIR}/include/object.h"
  "${CMAKE_CURRENT_LIST_DIR}/include/parser.h"
  "${CMAKE_CURRENT_LIST_DIR}/include/pstdlib.h"
  "${CMAKE_CURRENT_LIST_DIR}/include/strescape.h"
  "${CMAKE_CURRENT_LIST_DIR}/include/token.h"
  "${CMAKE_CURRENT_LIST_DIR}/include/ustring.h"
  "${CMAKE_CURRENT_LIST_DIR}/include/utils.h"

  # External
  "${CMAKE_CURRENT_LIST_DIR}/external/stb/stb_ds.h"
)

set(PANKTI_GFX_SRC_FILES
  "${CMAKE_CURRENT_LIST_DIR}/external/kb/kbts_impl.c"
  "${CMAKE_CURRENT_LIST_DIR}/stdlib/stdgfx.c"
  "${CMAKE_CURRENT_LIST_DIR}/stdlib/gfxhelper/gfxfont.c"
  "${CMAKE_CURRENT_LIST_DIR}/stdlib/gfxhelper/gfxhelper.c"
  "${CMAKE_CURRENT_LIST_DIR}/stdlib/gfxhelper/gfxkeyboard.c"
)

set(PANKTI_MAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/main.c")
