#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#include "token.h"
#include "ustring.h"
#include <time.h>

typedef struct PanktiCore PanktiCore;

// The Lexer Object
typedef struct Lexer {
    // Script source code string
    char *source;
    // Raw Lexer (Lexer is given raw compile time string to work on)
    // If the lexer is raw, `source` will not be freed
    bool raw;
    // Length of source code string
    size_t length;
    // Array of tokens; Filled with ScanTokens(...) function
    Token **tokens;

    // Start index of the character for token to be created
    // When creating a new token, this will be the first character index
    size_t start;
    // Current index of the character being read
    // Will be last character of newly created token
    size_t current;
    // Current line number
    size_t line;
    // current column number of the start of the token
    size_t column;

    // Codepoint iterator. The characters are read as UTF-32 characters
    UIter *iter;

    // Timestamp seed for hashing
    uint64_t timestamp;

    // Lexer Error occured
    bool hasError;

    // Reference to Pankti Core
    PanktiCore *core;
} Lexer;

// Create new Lexer Object;
// `src` = source code
Lexer *NewLexer(char *src);

// Turn Lexer into Raw Lexer
// It means: Lexer is given raw compile time string to work on
// If the lexer is raw, `source` will not be freed
void MakeLexerRaw(Lexer *lexer, bool raw);

// Reset lexer state
// Change source before calling this function
void ResetLexer(Lexer *lexer);

// Free lexer
void FreeLexer(Lexer *lexer);

// Tokenize the Source Code; Adds to internal Token list;
// Returns pointer to internal token list; (Must not free)
Token **ScanTokens(Lexer *lexer);

#ifdef __cplusplus
}
#endif

#endif
