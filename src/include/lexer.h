#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include "ustring.h"
typedef struct Lexer{
	char * source;
	long length;
	Token ** tokens;

	long start;
	long current;
	long line;
	
	UIter * iter;

	void * core;
}Lexer;

//create new lexer;
//`src` -> source code
Lexer * NewLexer(char * src);
//free lexer
void FreeLexer(Lexer * lexer);
//tokenize the source code; adds to internal token list;
//returns pointer to internal token list; (Must not free)
Token ** ScanTokens(Lexer * lexer);

#endif
