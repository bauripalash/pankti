cmake_minimum_required(VERSION 3.24...3.30)

project(pankti)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(FetchContent)

option(
	USE_NAN_BOXING 
	"Enable NaN-Boxed Values (64bit pointers and IEEE-754 double must be present)" 
	ON
)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE 
		"Debug" 
		CACHE STRING 
		"Set Build Type (Debug, Release, RelWithDebInfo, MinSizeRel)" 
		FORCE)
endif()

set_property(CACHE 
	CMAKE_BUILD_TYPE 
	PROPERTY STRINGS 
	"Debug" "Release" "RelWithDebInfo" "MinSizeRel"
)

add_executable(${PROJECT_NAME})
add_subdirectory(src)

set(PANKTI_VERSION_FILE "${CMAKE_SOURCE_DIR}/PANKTI_VERSION.txt")

set(GIT_COMMIT_COUNT 1)
set(GIT_COMMIT_HASH "_")

find_package(Git QUIET)
if(GIT_FOUND)
	execute_process(
		COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		OUTPUT_VARIABLE GIT_LAST_TAG
		OUTPUT_STRIP_TRAILING_WHITESPACE
		ERROR_QUIET
		RESULT_VARIABLE TAG_RESULT
	)

	if(TAG_RESULT EQUAL 0)
		execute_process(
			COMMAND ${GIT_EXECUTABLE} rev-list --count ${GIT_LAST_TAG}..HEAD
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
			OUTPUT_VARIABLE GIT_COMMIT_COUNT
			OUTPUT_STRIP_TRAILING_WHITESPACE
			ERROR_QUIET
		)
	else()
		execute_process(
			COMMAND ${GIT_EXECUTABLE} rev-list --count HEAD
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
			OUTPUT_VARIABLE GIT_COMMIT_COUNT
			OUTPUT_STRIP_TRAILING_WHITESPACE
			ERROR_QUIET
		)
	endif()

	set(GIT_COMMIT_COUNT "\"${GIT_COMMIT_COUNT}\"")

	execute_process(
		COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		OUTPUT_VARIABLE GIT_COMMIT_HASH
		OUTPUT_STRIP_TRAILING_WHITESPACE
		ERROR_QUIET
	)

	message(STATUS "")
	message(STATUS "==== Pankti Version Info ====")
	message(STATUS "Git Hash : ${GIT_COMMIT_HASH}")
	message(STATUS "Git Count : ${GIT_COMMIT_COUNT}")
	message(STATUS "=============================")
	message(STATUS "")

	target_compile_definitions(${PROJECT_NAME} PRIVATE
		PANKTI_GIT_HASH="${GIT_COMMIT_HASH}"
		PANKTI_GIT_COUNT=${GIT_COMMIT_COUNT}
	)
endif()


enable_testing()
add_subdirectory(tests/frontend)

set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 11)
set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD_REQUIRED ON)
#set_property(TARGET ${PROJECT_NAME} PROPERTY C_EXTENSIONS OFF)

#target_link_options(${PROJECT_NAME} PRIVATE "-static")

set(PANKTI_DEBUG_CFLAGS
	"-g3" 
	"-pedantic"
	"-Wall" 
	"-Wconversion"
	"-fsanitize=address"
	"-fsanitize=undefined"
	"-fsanitize=float-cast-overflow" 
)

set(PANKTI_RELEASE_CFLAGS
	"-pedantic" 
	"-Wall"
	"-O2"
	"-flto" 
	"-funroll-loops" 
)

set (PANKTI_REL_WITH_DINFO_CFLAGS
	"-g3"
	${PANKTI_RELEASE_CFLAGS}
)

set(PANKTI_DEBUG_LINKER_FLAGS
	"-lm"
	"-fsanitize=address"
	"-fsanitize=undefined"
	"-fsanitize=float-cast-overflow"
	"-flto"
)

set(PANKTI_RELEASE_LINKER_FLAGS
	"-lm"
	"-flto"
	"-funroll-loops"
)

set(PANKTI_REL_WITH_DINFO_LINKER_FLAGS 
	${PANKTI_RELEASE_LINKER_FLAGS}
)



if(USE_NAN_BOXING)
	add_compile_definitions(USE_NAN_BOXING=1)
	message("NaN-Boxing is enabled")
else()
	message("NaN-Boxing is disabled")
endif()

target_compile_options(${PROJECT_NAME} PRIVATE 
	$<$<CONFIG:Debug>:${PANKTI_DEBUG_CFLAGS}>
	$<$<CONFIG:Release>:${PANKTI_RELEASE_CFLAGS}>
	$<$<CONFIG:RelWithDebInfo>:${PANKTI_REL_WITH_DINFO_CFLAGS}>
	$<$<CONFIG:MinSizeRel>:${PANKTI_RELEASE_CFLAGS}>
)

target_link_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:${PANKTI_DEBUG_LINKER_FLAGS}>
    $<$<CONFIG:Release>:${PANKTI_RELEASE_LINKER_FLAGS}>
    $<$<CONFIG:RelWithDebInfo>:${PANKTI_REL_WITH_DINFO_LINKER_FLAGS}>
    $<$<CONFIG:MinSizeRel>:${PANKTI_RELEASE_LINKER_FLAGS}>
)


if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_definitions(${PROJECT_NAME} PRIVATE MI_TRACK_ASAN=1)
	add_compile_definitions(PANKTI_BUILD_DEBUG)
else()
	add_compile_definitions(PANKTI_BUILD_RELEASE)
endif()


if (CMAKE_SYSTEM_NAME STREQUAL "Windows" OR CMAKE_SYSTEM_NAME STREQUAL "CYGWIN") 
	add_compile_definitions(PANTKI_OS_WIN)
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
	add_compile_definitions(PANKTI_OS_LINUX)
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin") 
	add_compile_definitions(PANKTI_OS_MAC)
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
	add_compile_definitions(PANKTI_OS_WEB)
endif()


