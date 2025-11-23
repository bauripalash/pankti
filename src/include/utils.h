#ifndef UTILS_H
#define UTILS_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <uchar.h>

#ifdef __cplusplus
extern "C" {
#endif

// Constant value of Pi used across the codebase
#define CONST_PI 3.14159265358979323846f
// Constant value of e used across the codebase
#define CONST_E            2.71828182845904523536f

#define MAX_STRFORMAT_BUF  4
#define MAX_STRSPLIT_COUNT 128
#define MAX_STRBUF_LENGTH  1024
// Return item count of array
#define ArrCount(arr) (sizeof(arr) / sizeof(arr[0]))

// Read file to string (must free the returned string)
// `path` = Path to the file
char *ReadFile(const char *path);

// Create a substring from `str` which is str[start...end];
// (must free the returned string)
// `str` = String which the substring will be created from
// `start` = Start index
// `end` = End index
char *SubString(const char *str, size_t start, size_t end);

// Return a malloc'd Duplicate string
char *StrDuplicate(const char *str, size_t len);

// String Hash?
uint64_t StrHash(const char *str, size_t len, uint64_t seed);

// Check if `str1` and `str2` is same
bool StrEqual(const char *str1, const char *str2);

// Get the length of a string. Return how many bytes, not actual codepoints
size_t StrLength(const char *str);

// Format String; same as Raylib's TextFormat
const char *StrFormat(const char *text, ...);

// Split String; same as Raylib's TextSplit
char **StrSplit(const char *text, char delimiter, int *count);

// Join Two String into one
char *StrJoin(const char *a, size_t alen, const char *b, size_t blen, bool *ok);

// Helper function for lexer to match English, Bengali and Phonetic version of
// keywords
// Return true if any of them matches
// `s` = Input string
// `en` = English keyword
// `pn` = Phonetic keyword
// `bn` = UTF-8 encoded Bengali keyword
bool MatchKW(const char *s, const char *en, const char *pn, const char *bn);

// Convert a single codepoint from UTF-8 to UTF-32 encoded character
// Will return invalid unicode if `str` is invalid.
// `str` = UTF-8 encoded string. Reads reads 1 to 4 bytes from string
char32_t U8ToU32(const unsigned char *str);

// Convert Bool to String; `"true"` or `"false"`
char *BoolToString(bool v);

double NumberFromStr(const char *lexeme, size_t len, bool *ok);

// Check if double is integer
bool IsDoubleInt(double d);

unsigned char ToHex2Bytes(char c1, char c2);
unsigned char HexStrToByte(char *str, int len);

#ifdef __cplusplus
}
#endif

#endif
