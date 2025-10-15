#ifndef PARSER_H
#define PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexer.h"
#include "token.h"
#include "ast.h"

// Parser Object
typedef struct Parser{
	// The Lexer
	Lexer * lx;
	// Array of Tokens, referenced from lexer's token array
	Token ** tokens;
	// Reference to PanktiCore
	void * core;
	// Reading Postion of token from token array
	int pos;

	// Array of Statements
	PStmt ** stmts;
	// Flag to check if error has occured
	bool hasError;
}Parser;

// Create New Parser Object
// `lexer` = The lexer object. The lexer must have already scanned the source
Parser * NewParser(Lexer * lexer);
// Free the Parser object
void FreeParser(Parser * parser);
// Run the parser. Return reference to internal statements array
PStmt ** ParseParser(Parser * parser);

#ifdef __cplusplus
}
#endif

#endif
