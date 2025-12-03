#ifndef PTYPES_H
#define PTYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
    #define finline __forceinline
#elif defined(__GNUC__)
    #define finline inline __attribute__((__always_inline__))
#elif defined(__CLANG__)
    #if __has_attribute(__always_inline__)
        #define finline inline __attribute__((__always_inline__))
    #else
        #define finline inline
    #endif
#else
    #define finline inline
#endif

#include <stdint.h>
// Unsigned 64bit integer
typedef uint64_t u64;
// Signed 64bit integer
typedef int64_t i64;

// Unsigned 32bit integer
typedef uint32_t u32;
// Signed 32bit integer
typedef int32_t i32;

// Unsigned 16bit integer
typedef uint16_t u16;
// Signed 16bit integer
typedef int16_t i16;

// Unsigned 8bit integer
typedef uint8_t u8;
// Signed 8bit integer
typedef int8_t i8;

// Unsigned char
typedef unsigned char uchar;
// Unsigned int
typedef unsigned int uint;

// UTF-32 Character
typedef uint32_t char32;
// UTF-16 Character
typedef uint16_t char16;
// UTF-8 Character
typedef uint8_t char8;

#ifdef __cplusplus
}
#endif

#endif
