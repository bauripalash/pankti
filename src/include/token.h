#ifndef TOKEN_H
#define TOKEN_H

#include <stdbool.h>

typedef enum TokenType{
	T_LEFT_PAREN,
	T_RIGHT_PAREN,
	T_LEFT_BRACE,
	T_RIGHT_BRACE,
	T_COMMA,
	T_DOT,
	T_PLUS,
	T_MINUS,
	T_SEMICOLON,
	T_SLASH,
	T_ASTR,

	T_BANG,
	T_BANG_EQ,
	T_EQ,
	T_EQEQ,
	T_GT,
	T_GTE,
	T_LT,
	T_LTE,

	T_IDENT,
	T_STR,
	T_NUM,

	T_LET,
	T_AND,
	T_OR,
	T_FUNC,
	T_IF,
	T_THEN,
	T_ELSE,
	T_END,

	T_EOF
}TokenType;

char * TokTypeToStr(TokenType type);
void PrintTokType(TokenType type);

typedef struct Token{
	TokenType type;
	char * lexeme;
	long line;
}Token;

Token * NewToken(TokenType type);
void FreeToken(Token * token);
bool SetTokenLexeme(Token * token, char * str);
void PrintToken(const Token * token);

#endif
