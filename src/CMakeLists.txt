set(PANKTI_SRC_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/ast.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/bengali.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/core.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/env.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/interpreter.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/lexer.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/main.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/native.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/object.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/parser.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/strescape.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/token.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/ustring.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/utils.c"

  "${CMAKE_CURRENT_SOURCE_DIR}/gc/gc.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/gc/gcexpr.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/gc/gcobject.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/gc/gcstmt.c"

  # Stdlib
  "${CMAKE_CURRENT_SOURCE_DIR}/stdlib/pstdlib.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/stdlib/stdmap.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/stdlib/stdmath.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/stdlib/stdos.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/stdlib/stdstring.c"

  # External
  "${CMAKE_CURRENT_SOURCE_DIR}/external/xxhash.c"
)

set(PANKTI_HEADER_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/include/alloc.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/ansicolors.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/ast.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/bengali.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/core.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/defaults.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/env.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/gc.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/interpreter.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/keywords.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/lexer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/object.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/parser.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/pstdlib.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/strescape.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/token.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/ustring.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/utils.h"

  # External
  "${CMAKE_CURRENT_SOURCE_DIR}/external/stb/stb_ds.h"
)

target_sources(${PROJECT_NAME} PRIVATE ${PANKTI_SRC_FILES} ${PANKTI_HEADER_FILES})
