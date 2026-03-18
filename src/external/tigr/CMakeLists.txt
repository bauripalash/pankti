cmake_minimum_required(VERSION 3.24)

add_library(tigr STATIC tigr.c)

target_include_directories(tigr PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if (WIN32 OR CMAKE_SYSTEM_NAME STREQUAL "Windows" OR 
	CMAKE_SYSTEM_NAME STREQUAL "CYGWIN" OR 
	CMAKE_SYSTEM_NAME STREQUAL "MSYS") 
    target_link_libraries(tigr PUBLIC opengl32 gdi32)
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(tigr PRIVATE GLU GL X11)
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin") 
    target_link_libraries(tigr PUBLIC
        "-framework OpenGL"
        "-framework Cocoa"
    )
endif()
