#ifndef PTYPES_H
#define PTYPES_H

#include <stddef.h>
#include <stdint.h>
#include <uchar.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int puint;

// Signed integer
typedef int pint;

// 8bit unsigned integer
typedef uint8_t pu8;

// 16bit unsigned integer
typedef uint16_t pu16;

// 32bit unsigned integer
typedef uint32_t pu32;

// 64bit unsigned integer
typedef uint64_t pu64;

// 8bit signed integer
typedef int8_t pint8;

// 16bit signed integer
typedef int16_t pint16;

// 32bit signed integer
typedef int32_t pint32;

// 64bit signed integer
typedef int64_t pint64;

// 16bit UTF-16 character
typedef char16_t pchar16;

// 32bit UTF-32 character
typedef char32_t pchar32;

// Unsigned character
typedef unsigned char puchar;

// Platform dependent counting, size_t unsigned integer type
typedef size_t pucount;

#ifdef __cplusplus
}
#endif

#endif
