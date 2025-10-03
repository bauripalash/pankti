#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/ucontext.h>
#include <uchar.h>

#include "include/lexer.h"
#include "include/bengali.h"
#include "include/token.h"
#include "include/ustring.h"
#include "include/utils.h"

#define STB_DS_IMPLEMENTATION
#include "external/stb/stb_ds.h"

Lexer * NewLexer(char * src){
	Lexer * lx = malloc(sizeof(Lexer));
	if (lx == NULL) {
		return NULL;
	}
	
	UIter * iter = NewUIterator(src);
	if (iter == NULL) {
		free(lx);
		return NULL;
	}

	lx->iter = iter;

	lx->current = 0;
	lx->start = 0;
	lx->line = 1;
	lx->source = src;
	lx->length = strlen(src);
	lx->tokens = NULL;

	return lx;
}
void FreeLexer(Lexer * lexer){
	if (lexer == NULL) {
		return;
	}

	if (lexer->source != NULL) {
		free(lexer->source);
	}

	if (lexer->tokens != NULL) {
		//Free Tokens;
	}

	if (lexer->iter != NULL) {
		FreeUIterator(lexer->iter);
	}

	free(lexer);
}

static inline bool isAnyNumber(char32_t c){
	return (c >= '0' && c <= '9') || IsBnNumber((c));
}

static inline bool isAnyAlpha(char32_t c){
	bool isEnAlpha = ((c >= 'a' && c <= 'z') ||
						(c >= 'A' && c <= 'Z') ||
				   			(c == '_')
				   			);

	bool isBnAlpha = IsBnChar(c) && !IsBnNumber(c);

	return isEnAlpha || isBnAlpha;
						
}

static inline bool isAnyAlphaNum(char32_t c){
	return isAnyAlpha(c) || isAnyNumber(c);
}

static bool atEnd(const Lexer * lexer){
	return UIterIsEnd(lexer->iter);
}

static char32_t advance(Lexer * lexer){
	size_t pos = lexer->iter->pos;
	char32_t cp = UIterNext(lexer->iter);
	if (cp != 0) {
		lexer->current += (lexer->iter->pos - pos);
		return cp;
	}

	return 0;
}

static char32_t _peek(const Lexer * lx, int n){
	if (atEnd(lx) || n < 0 ) {
		return 0;
	}
	return UIterPeek(lx->iter, n);
}

static char32_t peek(const Lexer * lx){
	return _peek(lx, 0);
}
static char32_t peekPeek(const Lexer * lx){
	return _peek(lx, 1);
}

static bool match(Lexer * lx, char32_t target){
	if (atEnd(lx)) {
		return false;
	}

	if (peek(lx) != target) {
		return false;
	}

	advance(lx);
	return true;
}



static bool addTokenWithLexeme(Lexer * lx, TokenType type, char * str){
	Token * tok = NewToken(type);
	if (tok == NULL) {
		return false;
	}

	tok->lexeme = str;
	arrput(lx->tokens, tok);
	return true;

}

static bool addToken(Lexer * lx, TokenType type){
	return addTokenWithLexeme(lx, type, NULL);
}

static void readString(Lexer * lx){
	while (peek(lx) != '"' && !atEnd(lx)) {
		if (peek(lx) != '\n') {
			lx->line++;
		}
		advance(lx);
	}

	advance(lx);

	char * lexeme = SubString(lx->source, lx->start + 1, lx->current - 1);
	addTokenWithLexeme(lx, T_STR, lexeme);
	
}


static void readNumber(Lexer * lx){
	while (isAnyNumber(peek(lx))) {
		advance(lx);
	}

	if (peek(lx) == '.' && isAnyNumber(peekPeek(lx))) {
		advance(lx);
		while (isAnyNumber(peek(lx))) {
			advance(lx);
		}
	}
	char * lexeme = SubString(lx->source, lx->start, lx->current);
	addTokenWithLexeme(lx, T_NUM, lexeme);
}

static void readIdent(Lexer * lx){
	while (isAnyAlphaNum(peek(lx))) {
		advance(lx);
	}

	char * lexeme = SubString(lx->source, lx->start, lx->current);
	addTokenWithLexeme(lx, T_IDENT, lexeme);
}


static void scanToken(Lexer * lx){
	char32_t c = advance(lx);
	switch (c) {
		case '(' : addToken(lx, T_LEFT_PAREN);break;
		case ')' : addToken(lx, T_RIGHT_PAREN);break;
		case '{': addToken(lx, T_LEFT_BRACE);break;
		case '}': addToken(lx, T_RIGHT_BRACE);break;
		case ',': addToken(lx, T_COMMA);break;
		case '.': addToken(lx, T_DOT);break;
		case '+': addToken(lx, T_PLUS);break;
		case '-': addToken(lx, T_MINUS);break;
		case '/': addToken(lx, T_SLASH);break;
		case '*': addToken(lx, T_ASTR);break;
		case ';': addToken(lx, T_SEMICOLON);break;
		case '!':{
			if (match(lx, '=')) {
				addToken(lx, T_BANG_EQ);
			}else{
				addToken(lx, T_BANG);
			}
			break;
		}

		case '=':{
			if (match(lx, '=')) {
				addToken(lx, T_EQEQ);
			}else{
				addToken(lx, T_EQ);
			}
			break;
		}

		case '<':{
			if (match(lx, '=')) {
				addToken(lx, T_LTE);
			}else{
				addToken(lx, T_LT);
			}
			break;
		}

		case '>':{
			if (match(lx, '=')) {
				addToken(lx, T_GTE);
			}else {
				addToken(lx, T_GT);
			}
			break;
		}
		case '"': readString(lx); break;
		case ' ':
		case '\r':
		case '\t':
			break;
		case '\n': lx->line++;break;
		default:{
			if (isAnyNumber(c)) {
				readNumber(lx);
			}else if (isAnyAlpha(c)){
				readIdent(lx);
			}else{
				printf("L%ld : Invalid character : 0x%x | '%c'\n", lx->line,c, c);
			}
			break;
		}
	}

}

Token ** ScanTokens(Lexer * lexer){
	while (!atEnd(lexer)) {
		lexer->start = lexer->current;
		scanToken(lexer);
		
	}

	return lexer->tokens;
}
