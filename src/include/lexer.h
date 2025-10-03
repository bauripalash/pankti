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
}Lexer;

Lexer * NewLexer(char * src);
void FreeLexer(Lexer * lexer);
Token ** ScanTokens(Lexer * lexer);

#endif
