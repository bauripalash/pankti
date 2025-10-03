#include "include/token.h"
#include "include/utils.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char * TokTypeToStr(TokenType type){
	switch (type) {
		case T_LEFT_PAREN: return "LeftParen";
		case T_RIGHT_PAREN: return "RightParen";
		case T_LEFT_BRACE: return "LeftBrace";
		case T_RIGHT_BRACE: return "RightBrace";
		case T_COMMA: return "Comma";
		case T_DOT: return "Dot";
		case T_PLUS: return "Plus";
		case T_MINUS: return "Minus";
		case T_SEMICOLON: return "Semicolon";
		case T_SLASH: return "Slash";
		case T_ASTR: return "Astr";
		case T_BANG: return "Bang";
		case T_BANG_EQ: return "BangEq";
		case T_EQ: return "Eq";
		case T_EQEQ: return "EqEq";
		case T_GT: return "Gt";
		case T_GTE: return "GtE";
		case T_LT: return "Lt";
		case T_LTE: return "LtE";
		case T_IDENT: return "Ident";
		case T_STR: return "Str";
		case T_NUM: return "Num";
		case T_LET: return "Let";
		case T_AND: return "And";
		case T_OR: return "Or";
		case T_FUNC: return "Fun";
		case T_IF: return "If";
		case T_THEN: return "Then";
		case T_ELSE: return "Else";
		case T_END: return "End";
		case T_EOF: return "EOF";

		default: return "Unknown";
	};
}
void PrintTokType(TokenType type){
	printf("%s", TokTypeToStr(type));
}

Token * NewToken(TokenType type){
	Token * tok = malloc(sizeof(Token));
	if (tok == NULL) {
		return NULL;
	}

	tok->lexeme = NULL;
	tok->type = type;
	tok->line = 0;

	return tok;
}



void FreeToken(Token * token){
	if (token == NULL) {
		return;
	}

	if (token->lexeme != NULL) {
		free(token->lexeme);
	}

	free(token);
}

bool SetTokenLexeme(Token * token, char * str){
	if (token == NULL || str == NULL) {
		return false;
	}

	token->lexeme = str;
	return true;
}

void PrintToken(const Token * token){
	printf("Token[%s:'%s'()][L%ld]", TokTypeToStr(token->type), token->lexeme != NULL ? token->lexeme : "[NULL]", token->line);
	if (token->type == T_IDENT) {
		printf("(");
		printf("0x%02x", U8ToU32((unsigned char*)token->lexeme));
		printf(")");
	}
}
