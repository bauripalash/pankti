#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <uchar.h>

#include "include/alloc.h"
#include "include/bengali.h"
#include "include/core.h"
#include "include/keywords.h"
#include "include/lexer.h"
#include "include/token.h"
#include "include/ustring.h"
#include "include/utils.h"

// #define STBDS_SIPHASH_2_4
#define STB_DS_IMPLEMENTATION
#include "external/stb/stb_ds.h"

Lexer *NewLexer(char *src) {
    Lexer *lx = PMalloc(sizeof(Lexer));
    if (lx == NULL) {
        return NULL;
    }

    UIter *iter = NewUIterator(src);
    if (iter == NULL) {
        PFree(lx);
        return NULL;
    }

    lx->iter = iter;

    lx->current = 0;
    lx->start = 0;
    lx->column = 1;
    lx->line = 1;
    lx->source = src;
    lx->length = strlen(src);
    lx->tokens = NULL;
    lx->core = NULL;
	lx->raw = false;

    return lx;
}

void MakeLexerRaw(Lexer * lexer , bool raw){
	if (lexer == NULL) {
		return;
	}
	lexer->raw = raw;
}

void FreeLexer(Lexer *lexer) {
    if (lexer == NULL) {
        return;
    }

	if (!lexer->raw) {
		if (lexer->source != NULL) {
	        PFree(lexer->source);
	    }
	}
    

    if (lexer->tokens != NULL) {
        long count = arrlen(lexer->tokens);
        for (long i = 0; i < count; i++) {
            FreeToken(arrpop(lexer->tokens));
        }
        arrfree(lexer->tokens);
    }

    if (lexer->iter != NULL) {
        FreeUIterator(lexer->iter);
    }

    PFree(lexer);
}

void ResetLexer(Lexer * lexer){
	if (lexer->tokens != NULL) {
	    long count = arrlen(lexer->tokens);
        for (long i = 0; i < count; i++) {
            FreeToken(arrpop(lexer->tokens));
        }
        arrfree(lexer->tokens);
	}

    if (lexer->iter != NULL) {
        FreeUIterator(lexer->iter);
    }


    UIter *iter = NewUIterator(lexer->source);
    lexer->iter = iter;

    lexer->current = 0;
    lexer->start = 0;
    lexer->column = 1;
    lexer->line = 1;
    lexer->length = strlen(lexer->source);
    lexer->tokens = NULL;
}

static inline bool isAnyNumber(char32_t c) {
    return (c >= '0' && c <= '9') || IsBnNumber((c));
}

static inline bool isAnyAlpha(char32_t c) {
    bool isEnAlpha =
        ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_'));

    bool isBnAlpha = IsBnChar(c) && !IsBnNumber(c);

    return isEnAlpha || isBnAlpha;
}

static inline bool isAnyAlphaNum(char32_t c) {
    return isAnyAlpha(c) || isAnyNumber(c);
}

static bool atEnd(const Lexer *lexer) { return UIterIsEnd(lexer->iter); }

static char32_t advance(Lexer *lexer) {
    size_t pos = lexer->iter->pos;
    char32_t cp = UIterNext(lexer->iter);
    if (cp != 0) {
        lexer->current += (lexer->iter->pos - pos);
        lexer->column += lexer->iter->pos - pos;
        return cp;
    }

    return 0;
}

static char32_t _peek(const Lexer *lx, int n) {
    if (atEnd(lx) || n < 0) {
        return 0;
    }
    return UIterPeek(lx->iter, n);
}

static char32_t peek(const Lexer *lx) { return _peek(lx, 0); }
static char32_t peekPeek(const Lexer *lx) { return _peek(lx, 1); }

static bool match(Lexer *lx, char32_t target) {
    if (atEnd(lx)) {
        return false;
    }

    if (peek(lx) != target) {
        return false;
    }

    advance(lx);
    return true;
}

static bool addTokenWithLexeme(Lexer *lx, TokenType type, char *str, size_t len) {
    Token *tok = NewToken(type);
    if (tok == NULL) {
        return false;
    }

    tok->lexeme = str;
    tok->line = lx->line;
    if (str != NULL) {
        tok->len = len;
        tok->hash = StrHash(str, len, lx->timestamp);
    } else {
        if (IsDoubleTok(type)) {
            tok->len = 2;
        } else {
            tok->len = 1;
        }
    }
    size_t column = lx->column;

    column -= tok->len;

    tok->col = column;
    arrput(lx->tokens, tok);
    return true;
}

static bool addToken(Lexer *lx, TokenType type) {
    return addTokenWithLexeme(lx, type, NULL, 0);
}

static bool addStringToken(
    Lexer *lx, char *str, size_t line, size_t col, size_t len
) {
    Token *tok = NewToken(T_STR);
    if (tok == NULL) {
        return false;
    }

    tok->lexeme = str;
    tok->line = line;
    tok->col = col;
    tok->len = len - 2;
    tok->hash = StrHash(str, tok->len, lx->timestamp);
    arrput(lx->tokens, tok);
    return true;
}

static void readString(Lexer *lx) {
    size_t column = lx->column - 1;
    size_t line = lx->line;
    while (peek(lx) != '"' && !atEnd(lx)) {
        if (peek(lx) == '\n') {
            lx->line++;
            lx->column = 1;
        } else if (peek(lx) == '\\') {
            advance(lx);
        }
        advance(lx);
    }

    advance(lx);

    char *lexeme = SubString(lx->source, lx->start + 1, lx->current - 1);
    addStringToken(lx, lexeme, line, column, lx->current - lx->start);
}

static void readNumber(Lexer *lx) {
    while (isAnyNumber(peek(lx))) {
        advance(lx);
    }

    if (peek(lx) == '.' && isAnyNumber(peekPeek(lx))) {
        advance(lx);
        while (isAnyNumber(peek(lx))) {
            advance(lx);
        }
    }
    char *lexeme = SubString(lx->source, lx->start, lx->current);
    addTokenWithLexeme(lx, T_NUM, lexeme, lx->current - lx->start);
}

static TokenType getIdentType(const char *str) {
    if (MatchKW(str, KW_EN_LET, KW_PN_LET, KW_BN_LET)) {
        return T_LET;
    } else if (MatchKW(str, KW_EN_AND, KW_PN_AND, KW_BN_AND)) {
        return T_AND;
    } else if (MatchKW(str, KW_EN_OR, KW_PN_OR, KW_BN_OR)) {
        return T_OR;
    } else if (MatchKW(str, KW_EN_END, KW_PN_END, KW_BN_END)) {
        return T_END;
    } else if (MatchKW(str, KW_EN_IF, KW_PN_IF, KW_BN_IF)) {
        return T_IF;
    } else if (MatchKW(str, KW_EN_THEN, KW_PN_THEN, KW_BN_THEN)) {
        return T_THEN;
    } else if (MatchKW(str, KW_EN_ELSE, KW_PN_ELSE, KW_BN_ELSE)) {
        return T_ELSE;
    } else if (MatchKW(str, KW_EN_WHILE, KW_PN_WHILE, KW_BN_WHILE)) {
        return T_WHILE;
    } else if (MatchKW(str, KW_EN_NIL, KW_PN_NIL, KW_BN_NIL)) {
        return T_NIL;
    } else if (MatchKW(str, KW_EN_TRUE, KW_PN_TRUE, KW_BN_TRUE)) {
        return T_TRUE;
    } else if (MatchKW(str, KW_EN_FALSE, KW_PN_FALSE, KW_BN_FALSE)) {
        return T_FALSE;
    } else if (MatchKW(str, KW_EN_RETURN, KW_PN_RETURN, KW_BN_RETURN)) {
        return T_RETURN;
    } else if (MatchKW(str, KW_EN_FUNC, KW_PN_FUNC, KW_BN_FUNC)) {
        return T_FUNC;
    } else if (MatchKW(str, KW_EN_IMPORT, KW_PN_IMPORT, KW_BN_IMPORT)) {
        return T_IMPORT;
    } else if (MatchKW(str, KW_EN_PANIC, KW_PN_PANIC, KW_BN_PANIC)) {
        return T_PANIC;
    } else if (MatchKW(str, KW_EN_DO, KW_PN_DO, KW_BN_DO)) {
        return T_DO;
    } else if (MatchKW(str, KW_EN_BREAK, KW_PN_BREAK, KW_BN_BREAK)) {
        return T_BREAK;
    } else if (MatchKW(str, KW_EN_LEN, KW_PN_LEN, KW_BN_LEN)) {
        return T_LEN;
    } else if (MatchKW(str, KW_EN_PRINT, KW_PN_PRINT, KW_BN_PRINT)) {
        return T_PRINT;
    }

    return T_IDENT;
}

static void readIdent(Lexer *lx) {
    while (isAnyAlphaNum(peek(lx))) {
        advance(lx);
    }

    char *lexeme = SubString(lx->source, lx->start, lx->current);
    TokenType identType = getIdentType(lexeme);
    addTokenWithLexeme(lx, identType, lexeme, lx->current - lx->start);
}

static void scanToken(Lexer *lx) {
    char32_t c = advance(lx);
    switch (c) {
        case '(': addToken(lx, T_LEFT_PAREN); break;
        case ')': addToken(lx, T_RIGHT_PAREN); break;
        case '{': addToken(lx, T_LEFT_BRACE); break;
        case '}': addToken(lx, T_RIGHT_BRACE); break;
        case '[': addToken(lx, T_LS_BRACKET); break;
        case ']': addToken(lx, T_RS_BRACKET); break;
        case ',': addToken(lx, T_COMMA); break;
        case '.': addToken(lx, T_DOT); break;
        case '+': addToken(lx, T_PLUS); break;
        case '-': addToken(lx, T_MINUS); break;
        case '/': addToken(lx, T_SLASH); break;
        case ':': addToken(lx, T_COLON); break;
        case '%': addToken(lx, T_MOD); break;
        case ';': addToken(lx, T_SEMICOLON); break;
        case '*': {
            if (match(lx, '*')) {
                addToken(lx, T_EXPONENT);
            } else {
                addToken(lx, T_ASTR);
            }

            break;
        }
        case '!': {
            if (match(lx, '=')) {
                addToken(lx, T_BANG_EQ);
            } else {
                addToken(lx, T_BANG);
            }
            break;
        }

        case '=': {
            if (match(lx, '=')) {
                addToken(lx, T_EQEQ);
            } else {
                addToken(lx, T_EQ);
            }
            break;
        }

        case '<': {
            if (match(lx, '=')) {
                addToken(lx, T_LTE);
            } else {
                addToken(lx, T_LT);
            }
            break;
        }

        case '>': {
            if (match(lx, '=')) {
                addToken(lx, T_GTE);
            } else {
                addToken(lx, T_GT);
            }
            break;
        }
        case '"': readString(lx); break;
        case ' ': break;
        case '\r': break;
        case '\t': break;
        case '\n': {
            lx->line++;
            lx->column = 1;
            break;
        }
        case '#': {
            while (peek(lx) != '\n' && !atEnd(lx)) {
                advance(lx);
            }
            break;
        }
        default: {
            if (isAnyNumber(c)) {
                readNumber(lx);
                break;
            } else if (isAnyAlpha(c)) {
                readIdent(lx);
                break;
            } else {
				if (lx->core != NULL) {
					CoreLexerError(
						lx->core, lx->line, lx->column, "Invalid character found"
					);
				} else {
					printf("Invalid character found at line %ld column %ld", lx->line, lx->column);
				}
               
                break;
            }
            break;
        }
    }
}

Token **ScanTokens(Lexer *lexer) {
    while (!atEnd(lexer)) {
        lexer->start = lexer->current;
        scanToken(lexer);
    }

    addToken(lexer, T_EOF);

    return lexer->tokens;
}
