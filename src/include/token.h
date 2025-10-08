#ifndef TOKEN_H
#define TOKEN_H

#include <stdbool.h>
#include <stdint.h>

typedef enum TokenType{
	T_LEFT_PAREN,
	T_RIGHT_PAREN,
	T_LEFT_BRACE,
	T_RIGHT_BRACE,
	T_LS_BRACKET,
	T_RS_BRACKET,
	T_COMMA,
	T_DOT,
	T_PLUS,
	T_MINUS,
	T_COLON,
	T_MOD,
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
	T_WHILE,
	T_DO,
	T_BREAK,
	T_NIL,
	T_TRUE,
	T_FALSE,
	T_RETURN,
	T_IMPORT,
	T_PANIC,
	T_LEN,
	T_PRINT,

	T_EOF
}TokenType;

char * TokTypeToStr(TokenType type);
void PrintTokType(TokenType type);
bool IsDoubleTok(TokenType type);

typedef struct Token{
	TokenType type;
	char * lexeme;
	long line;
	long col;
	long len;
	uint32_t hash;
}Token;

Token * NewToken(TokenType type);
void FreeToken(Token * token);
bool SetTokenLexeme(Token * token, char * str);
void PrintToken(const Token * token);
void PrintOpToken(const Token * token);
#endif
