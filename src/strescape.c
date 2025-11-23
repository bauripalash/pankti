#include "include/strescape.h"
#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

static int hexToInt(char c) {
    if (isxdigit(c)) {
        if (isdigit(c)) {
            return c - '0';
        } else if (isalpha(c)) {
            return tolower(c) - 'a' + 10;
        } else {
            return -1;
        }
    } else {
        return -1;
    }
}

static inline StrEscapeErr parseNHex(
    const char *input, pusize slen, pusize *ri, pusize n, pu32 *out
) {
    if (*ri + n >= slen) {
        return SESC_INPUT_FINISHED_EARLY;
    }
    pu32 val = 0;
    for (pusize k = 0; k < n; k++) {
        char c = input[*ri + 1 + k];
        pu32 digit = (pu32)hexToInt(c);
        if (digit == -1) {
            return SESC_INVALID_HEX_CHAR; // Invalid Hex Digits
        }

        val = (val << 4) | digit;
    }

    *ri += n;
    *out = val;
    return SESC_OK; // Success
}

// push codepoint `cp` to `str`, starting from write index `wi`
// Buffer `str` should alaways have enough space, but just in case we check here
// for if something goes wrong with output size
static inline StrEscapeErr pushCodepoint(
    char *output, pusize *wi, pu32 cp, pusize outlen
) {
    char *p = output + *wi;
    if (cp <= 0x7F) {
        if (*wi + 1 >= outlen) {
            return SESC_BUFFER_NOT_ENOUGH;
        }
        p[0] = (char)cp;
        *wi = *wi + 1;
    } else if (cp <= 0x7FF) {
        if (*wi + 2 >= outlen) {
            return SESC_BUFFER_NOT_ENOUGH;
        }
        p[0] = (char)(0xC0 | ((cp >> 6) & 0x1F));
        p[1] = (char)(0x80 | (cp & 0x3F));
        *wi = *wi + 2;
    } else if (cp <= 0xFFFF) {
        if (*wi + 3 >= outlen) {
            return SESC_BUFFER_NOT_ENOUGH;
        }
        p[0] = (char)(0xE0 | ((cp >> 12) & 0x0F));
        p[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
        p[2] = (char)(0x80 | (cp & 0x3F));
        *wi = *wi + 3;
    } else {
        if (*wi + 4 >= outlen) {
            return SESC_BUFFER_NOT_ENOUGH;
        }
        p[0] = (char)(0xF0 | ((cp >> 18) & 0x07));
        p[1] = (char)(0x80 | ((cp >> 12) & 0x3F));
        p[2] = (char)(0x80 | ((cp >> 6) & 0x3F));
        p[3] = (char)(0x80 | (cp & 0x3F));
        *wi = *wi + 4;
    }
    return SESC_OK;
}

StrEscapeErr ProcessStringEscape(
    const char *input, pusize inlen, char *output, pusize outlen
) {
    if (input == NULL || output == NULL) {
        return SESC_NULL_PTR; // Should Never happen
    }

    // Input Read Index
    pusize ri = 0;
    // Output Write Index
    pusize wi = 0;
    StrEscapeErr err = SESC_OK;

    while (ri < inlen || input[ri] != '\0') {
        if (wi >= outlen - 1) {
            // We must have space for 1 char and '\0' NULL terminator
            return SESC_BUFFER_NOT_ENOUGH; // Buffer size not enough
        }

        char c = input[ri];

        if (c != '\\') {
            // Not a Escape character
            output[wi++] = c;
            ri++;
            continue;
        }

        if (ri + 1 >= inlen || input[ri + 1] == '\0') {
            return SESC_INPUT_FINISHED_EARLY;
        }

        ri++;                // eat the '\'
        char ec = input[ri]; // the actual escape character
        switch (ec) {
            case 'n': output[wi++] = '\n'; break;
            case 'a': output[wi++] = '\a'; break;
            case 'b': output[wi++] = '\b'; break;
            case 'f': output[wi++] = '\f'; break;
            case 'r': output[wi++] = '\r'; break;
            case 't': output[wi++] = '\t'; break;
            case 'v': output[wi++] = '\v'; break;
            case 'x': {
                pu32 val = 0;
                err = parseNHex(input, inlen, &ri, 2, &val);
                if (err != SESC_OK) {
                    return err;
                }

                output[wi++] = (char)val;
                break;
            }

            case 'u': {
                pu32 val = 0;
                err = parseNHex(input, inlen, &ri, 4, &val);
                if (err != SESC_OK) {
                    return err;
                }

                // val is high surrogate. We should find a low surrogate
                if (val >= 0xD800 && val <= 0xDBFF) {
                    if (ri + 2 >= inlen || input[ri + 1] != '\\' ||
                        input[ri + 2] != 'u') {
                        // \uHHHH not found
                        return SESC_NO_LOW_SURROGATE;
                    }

                    ri += 2; // eat '\' and 'u'

                    pu32 low = 0;
                    err = parseNHex(input, inlen, &ri, 4, &low);
                    if (err != SESC_OK) {
                        return err;
                    }

                    if (low < 0xDC00 || low > 0xDFFF) {
                        return SESC_INVALID_LOW_SURROGATE;
                    }
                    pu32 high = val;
                    pu32 combCp = 0x10000;
                    combCp += (high - 0xD800) << 10;
                    combCp += (low - 0xDC00);
                    val = combCp;

                } else if ((val >= 0xDC00) && (val <= 0xDFFF)) {
                    return SESC_LONE_LOW_SURROGATE;
                } // end low surrogate check

                // Either Val is valid codepoint without needing a low surrogate
                // or We have combined high surrogate and low surrogate
                err = pushCodepoint(output, &wi, val, outlen);
                if (err != SESC_OK) {
                    return err;
                }
                break;

            } // end \uHHHH check
            case 'U': {
                pu32 val = 0;
                err = parseNHex(input, inlen, &ri, 8, &val);
                if (err != SESC_OK) {
                    return err;
                }
                if (val > 0x10FFFF || (val >= 0xD800 && val <= 0xDFFF)) {
                    return SESC_8_INVALID_CP;
                }

                err = pushCodepoint(output, &wi, val, outlen);
                if (err != SESC_OK) {
                    return err;
                }
                break;

            } // end \U<HHHHHHHH> check
            default: {
                output[wi++] = ec;
                return SESC_UNKNOWN_ESCAPE;
            }
        } // end switch

        ri++;
    } // end while loop

    output[wi] = '\0';
    return SESC_OK;
}
