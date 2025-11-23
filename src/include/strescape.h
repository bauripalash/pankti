#ifndef STRESCAPE_H
#define STRESCAPE_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>
#define ERROR_UNICODE_CP 0xFFFD
// String Escape Errors
typedef enum StrEscapeErr {
    // No Error
    SESC_OK,
    // Unknown escape found, or something that we don't handle yet
    SESC_UNKNOWN_ESCAPE,
    // Char is not a hex digit
    SESC_INVALID_HEX_CHAR,
    // Output Size is small
    SESC_BUFFER_NOT_ENOUGH,
    // Input ended but hex still incomplete
    SESC_INPUT_FINISHED_EARLY,
    // No low surrogate was found
    SESC_NO_LOW_SURROGATE,
    // we found a lone low surrogate
    SESC_LONE_LOW_SURROGATE,
    // we found a low surrogate but it was invalid
    SESC_INVALID_LOW_SURROGATE,
    // Invalid codepoint in \U<HHHHHHHH>
    SESC_8_INVALID_CP,
    // Input or Output was NULL
    // should never happen
    SESC_NULL_PTR
} StrEscapeErr;

// Escape a string for escape characters like \n, \t \xHH, \uHHHH etc
// `input` = Input string (usually lexeme for parser)
// `inlen` = Length of Input string `input`
// `output` = Output buffer to write the escaped string to
// `outlen` = Length of Output buffer `output`
// Returns => SESC_OK if nothing goes wrong or other errors from `StrEscapeErr`
StrEscapeErr ProcessStringEscape(
    const char *input, size_t inlen, char *output, size_t outlen
);
#ifdef __cplusplus
}
#endif

#endif
