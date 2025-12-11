cmake_minimum_required(VERSION 3.24...3.30)

project(pankti C)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(FetchContent)

option(
	USE_NAN_BOXING 
	"Enable NaN-Boxed Values (Using IEEE-754 double)" 
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

set(GIT_COMMIT_HASH "_")
set(GIT_COMMIT_COUNT "_")

set(IS_OS_WIN FALSE)
set(IS_OS_LINUX FALSE)
set(IS_OS_MACOS FALSE)
set(IS_OS_WEB FALSE)

set(IS_GCC FALSE)
set(IS_CLANG FALSE)
set(IS_MSVC FALSE)
set(IS_EMCC FALSE)

set(IS_DEBUG_BUILD FALSE)
set(IS_RELEASE_BUILD FALSE)



# ================== GIT INFO CHECK ==================

include(cmake/getgitinfo.cmake)
GetGitInfo(GIT_COMMIT_HASH GIT_COMMIT_COUNT)

if (NOT (GIT_COMMIT_HASH STREQUAL "_"))
	add_compile_definitions(PANKTI_GIT_HASH="${GIT_COMMIT_HASH}")
endif()

if (NOT (GIT_COMMIT_COUNT STREQUAL "_"))
	add_compile_definitions(PANKTI_GIT_COUNT=${GIT_COMMIT_COUNT})
endif()

# ================== END GIT INFO CHECK ==================


# ================== OS CHECK ==================

if (CMAKE_SYSTEM_NAME STREQUAL "Windows" OR CMAKE_SYSTEM_NAME STREQUAL "CYGWIN") 
	set(IS_OS_WIN TRUE)
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set(IS_OS_LINUX TRUE)
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin") 
	set(IS_OS_MACOS TRUE)
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
	set(IS_OS_WEB TRUE)
	set(IS_EMCC TRUE)
endif()

# ================== END OS CHECK ==================


# ================== COMPILER CHECK ==================

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
	set(IS_GCC TRUE)
elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "AppleClang")
	set(IS_CLANG TRUE)
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
	set(IS_MSVC TRUE)
endif()


if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(IS_DEBUG_BUILD TRUE)
else()
	set(IS_RELEASE_BUILD TRUE)
endif()

# ================== END COMPILER CHECK ==================
set_property(GLOBAL PROPERTY C_STANDARD 11)
set_property(GLOBAL PROPERTY C_STANDARD_REQUIRED ON)

if (IS_GCC)
	set_property(GLOBAL PROPERTY C_EXTENSIONS ON)
endif()
#target_link_options(${PROJECT_NAME} PRIVATE "-static")




# ================== COMPILER FLAGS ==================
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

if(IS_MSVC)
	# Put MSVC CFLAGS overrides
endif()

# ================== END COMPILER FLAGS ==================

# ================== LINKER FLAGS ==================
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

if(IS_MSVC)
	# Put MSVC LDFLAGS overrides
endif()
# ================== END LINKER FLAGS ==================


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

if(IS_DEBUG_BUILD)
	target_compile_definitions(${PROJECT_NAME} PRIVATE MI_TRACK_ASAN=1)
	add_compile_definitions(PANKTI_BUILD_DEBUG)
else()
	add_compile_definitions(PANKTI_BUILD_RELEASE)
endif()

if(USE_NAN_BOXING)
	add_compile_definitions(USE_NAN_BOXING=1)
endif()

if (IS_OS_WIN) 
	add_compile_definitions(PANTKI_OS_WIN)
elseif (IS_OS_LINUX)
	add_compile_definitions(PANKTI_OS_LINUX)
elseif (IS_OS_MACOS) 
	add_compile_definitions(PANKTI_OS_MAC)
elseif (IS_OS_WEB)
	add_compile_definitions(PANKTI_OS_WEB)
endif()

enable_testing()
add_subdirectory(tests/frontend)

message(STATUS "")
message(STATUS "")
message(STATUS "======== Pankti Build Information ========")
message(STATUS "Git Hash : ${GIT_COMMIT_HASH}")
message(STATUS "Git Count : ${GIT_COMMIT_COUNT}")
if(USE_NAN_BOXING)
	message(STATUS "NaN-Boxing : Enabled")
else()
	message(STATUS "NaN-Boxing : Disabled")
endif()
message(STATUS "Build Type : ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler :  ${CMAKE_C_COMPILER_ID}")

message(STATUS "==========================================")
message(STATUS "")
message(STATUS "")

