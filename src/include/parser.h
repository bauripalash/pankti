#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "token.h"
#include "ast.h"

typedef struct Parser{
	Lexer * lx;
	Token ** tokens;
	void * core;
	int pos;

	PStmt ** stmts;
	bool hasError;
}Parser;

Parser * NewParser(Lexer * lexer);
void FreeParser(Parser * parser);
PStmt ** ParseParser(Parser * parser);

#endif
