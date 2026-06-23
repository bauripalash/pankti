/*
 * Copyright (c) 2022 Palash Bauri
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PARSER_H
#define PARSER_H

#include "diagonctx.h"
#include "gc.h"
#ifdef __cplusplus
extern "C" {
#endif

#include "ast.h"
#include "lexer.h"
#include "token.h"

// Parser Object
typedef struct Parser {
    // The Lexer
    Lexer *lx;
    // Array of Tokens, referenced from lexer's token array
    Token **tokens;
    PDiagonCtx errCtx;
    // Reference to GC
    Pgc *gc;
    // Reading Position of token from token array
    int pos;

    // Array of Statements
    PStmt **stmts;
    // Flag to check if error has occurred
    bool hasError;
} Parser;

// Create New Parser Object
// `lexer` = The lexer object. The lexer must have already scanned the source
Parser *NewParser(Pgc *gc, Lexer *lexer);
// Free the Parser object
void FreeParser(Parser *parser);
// Run the parser. Return reference to internal statements array
PStmt **ParseParser(Parser *parser);

#ifdef __cplusplus
}
#endif

#endif
