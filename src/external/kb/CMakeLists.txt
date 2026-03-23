cmake_minimum_required(VERSION 3.24)

add_library(kb STATIC kb_impl.c)

target_include_directories(kb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(kb PRIVATE -w)
