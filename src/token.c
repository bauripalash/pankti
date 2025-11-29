#include "include/token.h"
#include "include/alloc.h"
#include "include/ansicolors.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *TokTypeToStr(TokenType type) {
    switch (type) {
        case T_LEFT_PAREN: return "LeftParen";
        case T_RIGHT_PAREN: return "RightParen";
        case T_LEFT_BRACE: return "LeftBrace";
        case T_RIGHT_BRACE: return "RightBrace";
        case T_LS_BRACKET: return "LSBracket";
        case T_RS_BRACKET: return "RSBracket";
        case T_COMMA: return "Comma";
        case T_DOT: return "Dot";
        case T_PLUS: return "Plus";
        case T_MINUS: return "Minus";
        case T_COLON: return "Colon";
        case T_MOD: return "Mod";
        case T_SEMICOLON: return "Semicolon";
        case T_SLASH: return "Slash";
        case T_ASTR: return "Astr";
        case T_EXPONENT: return "Exponent";
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
        case T_FUNC: return "Func";
        case T_IF: return "If";
        case T_THEN: return "Then";
        case T_ELSE: return "Else";
        case T_END: return "End";
        case T_WHILE: return "While";
        case T_DO: return "Do";
        case T_BREAK: return "Break";
        case T_NIL: return "Nil";
        case T_TRUE: return "True";
        case T_FALSE: return "False";
        case T_RETURN: return "Return";
        case T_IMPORT: return "Import";
        case T_PANIC: return "Panic";
        case T_PRINT: return "Print";
        case T_EOF: return "EOF";
    };

    return "Unknown";
}
void PrintTokType(TokenType type) { printf("%s", TokTypeToStr(type)); }

bool IsDoubleTok(TokenType type) {
    if (type == T_BANG_EQ || type == T_EQEQ || type == T_GTE || type == T_LTE) {
        return true;
    }
    return false;
}

Token *NewToken(TokenType type) {
    Token *tok = PMalloc(sizeof(Token));
    if (tok == NULL) {
        return NULL;
    }

    tok->lexeme = NULL;
    tok->type = type;
    tok->line = 0;
    tok->hash = 0;

    return tok;
}

void FreeToken(Token *token) {
    if (token == NULL) {
        return;
    }

    if (token->lexeme != NULL) {
        PFree(token->lexeme);
    }

    PFree(token);
}

bool SetTokenLexeme(Token *token, char *str) {
    if (token == NULL || str == NULL) {
        return false;
    }

    token->lexeme = str;
    return true;
}

static const char *getLexeme(const Token *t) {
    if (t->lexeme != NULL) {
        return t->lexeme;
    }
    switch (t->type) {
        case T_LEFT_PAREN: return "(";
        case T_RIGHT_PAREN: return ")";
        case T_LEFT_BRACE: return "{";
        case T_RIGHT_BRACE: return "}";
        case T_LS_BRACKET: return "[";
        case T_RS_BRACKET: return "]";
        case T_COMMA: return ",";
        case T_DOT: return ".";
        case T_PLUS: return "+";
        case T_MINUS: return "-";
        case T_COLON: return ":";
        case T_MOD: return "%";
        case T_SEMICOLON: return ";";
        case T_SLASH: return "/";
        case T_ASTR: return "*";
        case T_EXPONENT: return "**";
        case T_BANG: return "!";
        case T_BANG_EQ: return "!=";
        case T_EQ: return "=";
        case T_EQEQ: return "==";
        case T_GT: return ">";
        case T_GTE: return ">=";
        case T_LT: return "<";
        case T_LTE: return "<=";
        case T_EOF: return "<EOF>";
        default: return "";
    }
}

void PrintToken(const Token *token) {
    printf(
        "Token[" TERMC_BLUE "%zu:%zu | " TERMC_PURPLE "%s" TERMC_BLUE
        ":'" TERMC_GREEN "%s" TERMC_BLUE "' (%ld)" TERMC_RESET "]",
        token->line, token->col, TokTypeToStr(token->type), getLexeme(token),
        token->len
    );
}

void PrintOpToken(const Token *token) {
    printf("[%s]", TokTypeToStr(token->type));
}
