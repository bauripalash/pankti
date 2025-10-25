#ifndef LEXER_H
#define LEXER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "token.h"
#include "ustring.h"

// The Lexer Object
typedef struct Lexer {
    // Script source code string
    char *source;
    // Length of source code string
    long length;
    // Array of tokens; Filled with ScanTokens(...) function
    Token **tokens;

    // Start index of the character for token to be created
    // When creating a new token, this will be the first character index
    long start;
    // Current index of the character being read
    // Will be last character of newly created token
    long current;
    // Current line number
    long line;
    // current column number of the start of the token
    long column;

    // Codepoint iterator. The characters are read as UTF-32 characters
    UIter *iter;

    // Reference to Pankti Core
    void *core;
} Lexer;

// Create new Lexer Object;
// `src` = source code
Lexer *NewLexer(char *src);

// Free lexer
void FreeLexer(Lexer *lexer);

// Tokenize the Source Code; Adds to internal Token list;
// Returns pointer to internal token list; (Must not free)
Token **ScanTokens(Lexer *lexer);

#ifdef __cplusplus
}
#endif

#endif
