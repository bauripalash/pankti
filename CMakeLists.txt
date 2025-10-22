cmake_minimum_required(VERSION 3.24...3.30)
project(pankti)

include(FetchContent)
set(CMAKE_EXPORT_COMPILE_COMMANDS)

add_executable(${PROJECT_NAME})
add_subdirectory(src)
set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 11)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g3 -Wall -fsanitize=undefined -fsanitize=float-cast-overflow" )
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined -fsanitize=float-cast-overflow")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall -O3" )


if (WIN32 OR MINGW OR MSYS OR CYGWIN) 
	add_compile_definitions(PANTKI_OS_WIN)
endif()

if (UNIX AND NOT APPLE AND NOT CYGWIN)
	add_compile_definitions(PANKTI_OS_LINUX)
endif()

if (APPLE) 
	add_compile_definitions(PANKTI_OS_MAC)
endif()
