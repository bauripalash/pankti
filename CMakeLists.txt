cmake_minimum_required(VERSION 3.24...3.30)

file(READ "${CMAKE_SOURCE_DIR}/PANKTI_VERSION.txt" PANKTI_BASE_VERSION)
string(STRIP "${PANKTI_BASE_VERSION}" PANKTI_BASE_VERSION)
set(PANKTI_VERSION "${PANKTI_BASE_VERSION}")
project(pankti VERSION ${PANKTI_BASE_VERSION})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(FetchContent)

find_package(Git QUIET)

if(GIT_FOUND)
	execute_process(
		COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		OUTPUT_VARIABLE GIT_COMMIT_HASH
		OUTPUT_STRIP_TRAILING_WHITESPACE
		ERROR_QUIET
	)
	execute_process(
		COMMAND ${GIT_EXECUTABLE} diff --quiet
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		RESULT_VARIABLE GIT_DIRTY
	)
	
	if (GIT_DIRTY)
		set(GIT_DIRTY_STR ".dirty")
	else()
		set(GIT_DIRTY_STR "")
	endif()

	execute_process(
		COMMAND ${GIT_EXECUTABLE} describe --tags --long
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		OUTPUT_VARIABLE GIT_DESCRIBE
		OUTPUT_STRIP_TRAILING_WHITESPACE
		ERROR_QUIET
		RESULT_VARIABLE GIT_DESCRIBE_RESULT
	)

	if(GIT_DESCRIBE_RESULT EQUAL 0)
		string(REGEX MATCH "^(.+)-([0-9]+)-g([0-9a-f]+)$" _ ${GIT_DESCRIBE})
		set(GIT_INFO_TAG ${CMAKE_MATCH_1})
		set(GIT_INFO_COUNT ${CMAKE_MATCH_2})
		set(GIT_INFO_HASH ${CMAKE_MATCH_3})
		if(GIT_INFO_COUNT EQUAL 0)
			# Tagged release
			set(PANKTI_VERSION "${GIT_INFO_TAG}${GIT_DIRTY_STR}")
		else()
			string(REGEX REPLACE "^v" "" VERSION_WITHOUT_V ${GIT_INFO_TAG})
			set(PANKTI_VERSION "${VERSION_WITHOUT_V}.dev.r${GIT_INFO_COUNT}+${GIT_INFO_HASH}${GIT_DIRTY_STR}")
		endif()
		
	else()
		execute_process(
			COMMAND ${GIT_EXECUTABLE} rev-list --count HEAD
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
			OUTPUT_VARIABLE GIT_COMMIT_COUNT
			OUTPUT_STRIP_TRAILING_WHITESPACE
			ERROR_QUIET
		)
		set(PANKTI_VERSION "${PANKTI_BASE_VERSION}.dev.r${GIT_COMMIT_COUNT}+${GIT_COMMIT_HASH}${GIT_DIRTY_STR}")
	endif()

else()
	set(PANKTI_VERSION "${PANKTI_BASE_VERSION}.dev+unknown")
endif()

option(USE_NAN_BOXING "Enable NaN-Boxed Values (64bit pointers and IEEE-754 double must be present)" ON)

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

enable_testing()
add_subdirectory(tests)

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

add_compile_definitions(PANKTI_VERSION="${PANKTI_VERSION}")

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


