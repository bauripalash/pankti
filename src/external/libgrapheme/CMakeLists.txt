cmake_minimum_required(VERSION 3.10)

# Static library libgrapheme
project(libgrapheme C)

set(LIBGRAPHEME_SRCS
    src/case.c
    src/character.c
    src/line.c
    src/sentence.c
    src/utf8.c
    src/util.c
    src/word.c
)

add_library(libgrapheme STATIC ${LIBGRAPHEME_SRCS})
set_target_properties(libgrapheme PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)
