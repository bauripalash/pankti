#include "include/parser.h"
#include "external/stb/stb_ds.h"
#include "include/alloc.h"
#include "include/ast.h"
#include "include/core.h"
#include "include/gc.h"
#include "include/lexer.h"
#include "include/token.h"
#include "include/utils.h"
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#define ERROR_UNICODE_CP 0xFFFD
// Create New Let Statement;
// Returns PStmt with type `STMT_LET`
static PStmt *rLet(Parser *p);

// Create New Statement
static PStmt *rStmt(Parser *p);

// Read assignment expression;
// Returns expression with type `EXPR_ASSIGN`
static PExpr *rAssignment(Parser *p);
static PExpr *rEquality(Parser *p);
static PExpr *rComparison(Parser *p);
static PExpr *rTerm(Parser *p);
static PExpr *rFactor(Parser *p);
static PExpr *rUnary(Parser *p);
static PExpr *rCall(Parser *p);
static PExpr *rPrimary(Parser *p);
static PExpr *rExpression(Parser *p);

// Check if parser has reached end
static bool atEnd(const Parser *p);

Parser *NewParser(Pgc *gc, Lexer *lexer) {
    Parser *parser = PCreate(Parser);
    // ERROR HANDLE
    parser->lx = lexer;
    parser->tokens = lexer->tokens;
    parser->gc = gc;
    parser->pos = 0;
    parser->core = NULL;
    parser->stmts = NULL;
    parser->hasError = false;
    return parser;
}
void FreeParser(Parser *parser) {
    if (parser == NULL) {
        return;
    }
    arrfree(parser->stmts);
    PFree(parser);
}

PStmt **ParseParser(Parser *parser) {
    while (!atEnd(parser)) {
        arrput(parser->stmts, rLet(parser));
    }

    return parser->stmts;
}

static Token *peek(const Parser *p) { return p->tokens[p->pos]; }

static bool atEnd(const Parser *p) { return peek(p)->type == T_EOF; }

static bool check(const Parser *p, TokenType t) {

    if (atEnd(p)) {
        return false;
    }
    return peek(p)->type == t;
}

static Token *previous(const Parser *p) { return p->tokens[p->pos - 1]; }

static Token *advance(Parser *p) {
    if (!atEnd(p)) {
        p->pos++;
    }

    return previous(p);
}

static bool matchMany(Parser *p, TokenType *types, int count) {
    for (int i = 0; i < count; i++) {
        if (check(p, types[i])) {
            advance(p);
            return true;
        }
    }

    return false;
}

static bool matchOne(Parser *p, TokenType type) {
    if (check(p, type)) {
        advance(p);
        return true;
    }

    return false;
}

static void error(Parser *p, Token *tok, char *msg) {
    p->hasError = true;
    CoreError(p->core, tok, msg);
}

static Token *eat(Parser *p, TokenType t, char *msg) {
    if (check(p, t)) {
        return advance(p);
    }

    error(p, peek(p), msg);
    return NULL;
}

static PExpr *rExpression(Parser *p) { return rAssignment(p); }

static PExpr *rAnd(Parser *p) {
    PExpr *expr = rEquality(p);
    while (matchOne(p, T_AND)) {
        Token *op = previous(p);
        PExpr *right = rEquality(p);
        expr = NewLogical(p->gc, expr, op, right);
    }
    return expr;
}

static PExpr *rOr(Parser *p) {
    PExpr *expr = rAnd(p);
    while (matchOne(p, T_OR)) {
        Token *op = previous(p);
        PExpr *right = rAnd(p);
        expr = NewLogical(p->gc, expr, op, right);
    }
    return expr;
}

static PExpr *rAssignment(Parser *p) {
    PExpr *expr = rOr(p);
    if (matchOne(p, T_EQ)) {
        Token *op = previous(p);
        PExpr *value = rAssignment(p);

        if (expr->type == EXPR_VARIABLE || expr->type == EXPR_SUBSCRIPT) {
            return NewAssignment(p->gc, op, expr, value);
        }

        error(p, NULL, "Invalid assignment");
        return NULL;
    }

    return expr;
}

static PExpr *rEquality(Parser *p) {
    PExpr *expr = rComparison(p);
    while (matchMany(p, (TokenType[]){T_BANG_EQ, T_EQEQ}, 2)) {
        Token *op = previous(p);
        PExpr *right = rComparison(p);

        expr = NewBinaryExpr(p->gc, expr, op, right);
    }

    return expr;
}

static PExpr *rComparison(Parser *p) {
    PExpr *expr = rTerm(p);
    while (matchMany(p, (TokenType[]){T_GT, T_GTE, T_LT, T_LTE}, 4)) {
        Token *op = previous(p);
        PExpr *right = rTerm(p);
        expr = NewBinaryExpr(p->gc, expr, op, right);
    }

    return expr;
}

static PExpr *rTerm(Parser *p) {
    PExpr *expr = rFactor(p);
    while (matchMany(p, (TokenType[]){T_MINUS, T_PLUS}, 2)) {
        Token *op = previous(p);
        PExpr *right = rFactor(p);
        expr = NewBinaryExpr(p->gc, expr, op, right);
    }

    return expr;
}

static PExpr *rFactor(Parser *p) {
    PExpr *expr = rUnary(p);
    while (matchMany(p, (TokenType[]){T_SLASH, T_ASTR}, 2)) {
        Token *op = previous(p);
        PExpr *right = rUnary(p);
        if (right == NULL) {
            error(p, NULL, "Invaid expression found in factor expression");
        }
        expr = NewBinaryExpr(p->gc, expr, op, right);
    }
    return expr;
}

static PExpr *rUnary(Parser *p) {
    if (matchMany(p, (TokenType[]){T_BANG, T_MINUS}, 2)) {
        Token *op = previous(p);
        PExpr *right = rUnary(p);
        return NewUnary(p->gc, op, right);
    }

    return rCall(p);
}

static PExpr *finishCallExpr(Parser *p, PExpr *expr) {
    PExpr **args = NULL;
    int count = 0;
    if (!check(p, T_RIGHT_PAREN)) {
        do {
            if (count >= 255) {
                error(p, NULL, "Can't have more than 255 arguments");
            }
            arrput(args, rExpression(p));
            count++;
        } while (matchOne(p, T_COMMA));
    }

    Token *rparen = eat(p, T_RIGHT_PAREN, "Expected ')' after arguments");
    return NewCallExpr(p->gc, rparen, expr, args, count);
}

static PExpr *rSubscript(Parser *p, PExpr *expr) {
    Token *op = previous(p);
    PExpr *indexExpr = rExpression(p);
    if (indexExpr == NULL) {
        error(p, op, "Invalid subscript index");
        return NULL;
    }
    eat(p, T_RS_BRACKET, "Expected ']' after subscript expression");
    return NewSubscriptExpr(p->gc, op, expr, indexExpr);
}

static PExpr *rCall(Parser *p) {
    PExpr *expr = rPrimary(p);
    while (true) {
        if (matchOne(p, T_LEFT_PAREN)) {
            expr = finishCallExpr(p, expr);
        } else if (matchOne(p, T_LS_BRACKET)) {
            expr = rSubscript(p, expr);
        } else {
            break;
        }
    }

    return expr;
}

static PExpr *rArrayExpr(Parser *p) {
    PExpr **items = NULL;

    while (!check(p, T_RS_BRACKET)) {
        arrput(items, rExpression(p));
        if (check(p, T_RS_BRACKET)) {
            break;
        }
        eat(p, T_COMMA, "Expected ',' comma after array item");
    }
    Token *rbrace = eat(p, T_RS_BRACKET, "Expected ']' after array items");
    return NewArrayExpr(p->gc, rbrace, items, arrlen(items));
}

static PExpr *rMapExpr(Parser *p) {
    PExpr **etable = NULL;
    Token *lbrace = previous(p);
    while (!check(p, T_RIGHT_BRACE)) {
        arrput(etable, rExpression(p));
        eat(p, T_COLON, "Expected ':' after map key");
        arrput(etable, rExpression(p));
        if (check(p, T_RIGHT_BRACE)) {
            break;
        }
        eat(p, T_COMMA, "Expected ',' after map pair");
    }
    eat(p, T_RIGHT_BRACE, "Expected '}' after map");
    return NewMapExpr(p->gc, lbrace, etable, arrlen(etable));
}

// push codepoint `cp` to `str`, starting from read index `rdi`
static inline size_t pushCodepoint(char * str, size_t rdi, uint32_t cp){
	char * p = str + rdi;
	if (cp <= 0x7F) {
		p[0] = (char)cp;
		return 1;
	} else if (cp <= 0x7FF){
		p[0] = (char)(0xC0 | ((cp >> 6) & 0x1F));
		p[1] = (char)(0x80 | (cp & 0x3F));
		return 2;
	}else if (cp <= 0xFFFF){
		p[0] = (char)(0xE0 | ((cp >> 12) & 0x0F));
		p[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
		p[2] = (char)(0x80 | (cp & 0x3F));
		return 3;
	}else{
		p[0] = (char)(0xF0 | ((cp >> 18) & 0x07));
		p[1] = (char)(0x80 | ((cp >> 12) & 0x3F));
		p[2] = (char)(0x80 | ((cp >> 6) & 0x3F));
		p[3] = (char)(0x80 | (cp & 0x3F));
		return 4;
	}
}

static inline uint32_t hxToInt(char c){
	if (isalpha(c)) {
		return (tolower(c) - 'a') + 10;
	} else if (isdigit(c)){
		return c - '0';
	}
	return 0;
}

static char * readStringEscapes(Parser * p, Token * op){
	char * raw = op->lexeme;
	size_t slen = strlen(raw);
	size_t scap = slen * 4 + 1;
	char * str = PCalloc(scap, sizeof(char));
	if (str == NULL) {
		return NULL;
	}

	size_t rdi = 0;
	for (size_t i = 0; i < slen; i++) {
		char c = raw[i];
		if (c == '\\' && i + 1 < slen) {
			i++;
			char ec = raw[i];
			switch (ec) {
				case '\\': str[rdi++] = '\\';break;
				case 'a': str[rdi++] = '\a';break;
				case 'b': str[rdi++] = '\b';break;
				case 'f': str[rdi++] = '\f';break;
				case 'n': str[rdi++] = '\n';break;
				case 'r': str[rdi++] = '\r';break;
				case 't': str[rdi++] = '\t';break;
				case 'v': str[rdi++] = '\v';break;
				// \xXX
				case 'x':{
					if (!(i + 2 < slen)) {
						error(p, op, "Incomplete \\xXX escape");
						rdi += pushCodepoint(str, rdi, ERROR_UNICODE_CP);
						break;
					}

					unsigned int xval = 0;

					if (isxdigit((unsigned char)raw[i+1]) && 
						isxdigit((unsigned char)raw[i+2])) {
						char xc = raw[++i];
						char xc2 = raw[++i];
						xval = ToHex2Bytes(xc, xc2);
						str[rdi++] = (char)xval;
						break;


					}else{
						error(p, op, "Invalid Hex digits found in string");
						rdi += pushCodepoint(str, rdi, ERROR_UNICODE_CP);
						break;
					}

					break;
				}
				// \uXXXX
				case 'u':{
					if (i + 4 < slen && 
						isxdigit((unsigned char)raw[i+1]) && 
						isxdigit((unsigned char)raw[i+2]) && 
						isxdigit((unsigned char)raw[i+3]) && 
						isxdigit((unsigned char)raw[i+4])) {

						// would be codepoint or hight surrogate
						uint32_t val = 0;
						for (int k = 1; k <= 4; k++) {
							char ch = raw[i + k];
							val = val * 16 + hxToInt(ch);	
						}
						i+=4;
						
						// If the val is in this range, there should be another
						// \uXXXX, if not we eat 
						if (val >= 0xD800 && val <= 0xDBFF) {
							// check if another \uXXXX sequence is found
							if (i + 6 < slen && 
								raw[i + 1] == '\\' &&
								raw[i + 2] == 'u' &&
								isxdigit((unsigned char)raw[i+3]) && 
								isxdigit((unsigned char)raw[i+4]) && 
								isxdigit((unsigned char)raw[i+5]) && 
								isxdigit((unsigned char)raw[i+6])) {
								
								uint32_t low = 0;
								for (int k = 3; k<=6; k++) {
									char ch = raw[i + k];
									low = low * 16 + hxToInt(ch);
								}

								// Algorithm seems to be
								// CP = (H - 0xD800 << 10) + (L - 0xDC00) + 0x10000
								if (low >= 0xDC00 && low <= 0xDFFF) {
									i+=6;
									uint32_t high = val;
									uint32_t combCp = 0x10000;
									combCp += (high - 0xD800) << 10;
									combCp += (low - 0xDC00);
									rdi += pushCodepoint(str, rdi, combCp);
								}else{
									error(p, op, "Invalid low surrogate for \\uXXXX sequence");
									rdi += pushCodepoint(str, rdi, ERROR_UNICODE_CP);
								}
							}else{
								// another /uXXXX should have been here, but was not found
								error(p, op, "Expected low surrogate for \\uXXXX sequence");
								rdi += pushCodepoint(str, rdi, ERROR_UNICODE_CP);
							}
						}else{
							// no surrogate needed
							rdi += pushCodepoint(str, rdi, val);
						}
					
					}else{
						error(p, op, "Invalid \\uXXXX sequence");
						rdi += pushCodepoint(str, rdi, ERROR_UNICODE_CP);
					}

					break;
				}
				case 'U':{
					if (i + 8 < slen && 
						isxdigit((unsigned char)raw[i+1]) && 
						isxdigit((unsigned char)raw[i+2]) && 
						isxdigit((unsigned char)raw[i+3]) && 
						isxdigit((unsigned char)raw[i+4]) && 
						isxdigit((unsigned char)raw[i+5]) && 
						isxdigit((unsigned char)raw[i+6]) && 
						isxdigit((unsigned char)raw[i+7]) && 
						isxdigit((unsigned char)raw[i+8])) {
						uint32_t val = 0;
						for (int k = 1; k <= 8; k++) {
							char ch = raw[i + k];
							val = val * 16  + hxToInt(ch);
						}

						i+=8;

						if (val > 0x10FFFF || (val >= 0xD800 && val <= 0xDFFF)) {
							error(p, op, "\\UXXXXXXXX escape sequence produced invalid codepoint");
							rdi += pushCodepoint(str, rdi, ERROR_UNICODE_CP);
						}else{
							rdi += pushCodepoint(str, rdi, val);
						}

					}else {
						error(p, op, "Invalid or Incomplete \\UXXXXXXXX sequence");
						rdi += pushCodepoint(str, rdi, ERROR_UNICODE_CP);
					}
					break;
				}
				default: str[rdi++] = ec; break;
			}
		
		}else{
			str[rdi] = c;
			rdi++;
		}
	}

	if (rdi <= scap) {
		str[rdi] = '\0';
	}else{
		str[scap] = '\0'; // do we need this?
	}

	return str;
}

static PExpr *rPrimary(Parser *p) {
    if (matchOne(p, T_TRUE)) {
        PExpr *e = NewLiteral(p->gc, previous(p), EXP_LIT_BOOL);
        e->exp.ELiteral.value.bvalue = true;
        return e;
    }

    if (matchOne(p, T_FALSE)) {
        PExpr *e = NewLiteral(p->gc, previous(p), EXP_LIT_BOOL);
        e->exp.ELiteral.value.bvalue = false;
        return e;
    }

    if (matchOne(p, T_NIL)) {
        return NewLiteral(p->gc, previous(p), EXP_LIT_NIL);
    }

    if (matchOne(p, T_NUM)) {
		Token * opTok = previous(p);

        bool ok = true;
        double value = NumberFromStr(opTok->lexeme, opTok->len, &ok);
		if (!ok) {
			error(p, opTok, "Invalid or malformed number found");
			return NULL;
		}

        PExpr * e = NewLiteral(p->gc, opTok, EXP_LIT_NUM);
        e->exp.ELiteral.value.nvalue = value;
		return e;
    }

    if (matchOne(p, T_STR)) {
		Token * opTok = previous(p);
		char * escapedStr = readStringEscapes(p, opTok);
        PExpr * e = NewLiteral(p->gc, previous(p), EXP_LIT_STR);
		
		e->exp.ELiteral.value.svalue = escapedStr;

		return e;
    }

    if (matchOne(p, T_IDENT)) {
        return NewVarExpr(p->gc, previous(p));
    }

    if (matchOne(p, T_LEFT_PAREN)) {
        Token *op = previous(p);
        PExpr *e = rExpression(p);
        eat(p, T_RIGHT_PAREN, "Expected ')'");
        return NewGrouping(p->gc, op, e);
    }

    if (matchOne(p, T_LS_BRACKET)) {
        return rArrayExpr(p);
    }

    if (matchOne(p, T_LEFT_BRACE)) {
        return rMapExpr(p);
    }

    error(p, previous(p), "Expected expression");
    return NULL;
}

static void syncParser(Parser *p) {
    advance(p);
    while (!atEnd(p)) {
        if (previous(p)->type == T_SEMICOLON) {
            return;
        }

        switch (peek(p)->type) {
            case T_FUNC:
            case T_LET:
            case T_WHILE:
            case T_IF:
            case T_RETURN:
            case T_IMPORT:
            case T_PRINT: return;
            default: advance(p); break;
        }
    }
}

static PStmt *rExprStmt(Parser *p) {
    PExpr *value = rExpression(p);
    return NewExprStmt(p->gc, previous(p), value);
}

static PStmt *rPrintStmt(Parser *p) {
    Token *op = previous(p);
    PExpr *value = rExpression(p);
    return NewPrintStmt(p->gc, op, value);
}

static PStmt *rLetStmt(Parser *p) {
    Token *name = eat(p, T_IDENT, "Expected Identifier");
    eat(p, T_EQ, "Expected equal");
    PExpr *value = rExpression(p);
    return NewLetStmt(p->gc, name, value);
}

static PStmt *rBlockStmt(Parser *p) {
    Token *curTok = previous(p);
    PStmt **stmtList = NULL;
    while (!check(p, T_RIGHT_BRACE) && !atEnd(p)) {
        arrput(stmtList, rLet(p));
    }

    eat(p, T_RIGHT_BRACE, "Expected '}' after block stmt");
    PStmt *block = NewBlockStmt(p->gc, curTok, stmtList);
    return block;
}

static PStmt *rToEndBlockStmt(Parser *p) {
    PStmt **stmtList = NULL;
    while (!check(p, T_END) && !atEnd(p)) {
        arrput(stmtList, rLet(p));
    }

    eat(p, T_END, "Expected 'end' after block");
    PStmt *block = NewBlockStmt(p->gc, previous(p), stmtList);
    return block;
}

static PStmt *rToEndOrElseBlockStmt(Parser *p, bool *hasElse) {
    PStmt **stmtList = NULL;

    while ((!check(p, T_END) && !check(p, T_ELSE)) && !atEnd(p)) {
        arrput(stmtList, rLet(p));
    }

    if (check(p, T_ELSE)) {
        eat(p, T_ELSE, "Expected 'else' ");
        *hasElse = true;
    } else if (check(p, T_END)) {
        eat(p, T_END, "Expected 'end'");
        *hasElse = false;
    }
    PStmt *block = NewBlockStmt(p->gc, previous(p), stmtList);
    return block;
}
static PStmt *rIfStmt(Parser *p) {
    Token *op = previous(p);
    PExpr *cond = rExpression(p);
    eat(p, T_THEN, "Expected then after if expression");
    bool hasElse = false;
    PStmt *thenBranch = rToEndOrElseBlockStmt(p, &hasElse);
    PStmt *elseBranch = NULL;
    if (hasElse) {
        elseBranch = rToEndBlockStmt(p);
    }

    return NewIfStmt(p->gc, op, cond, thenBranch, elseBranch);
}

static PStmt *rWhileStmt(Parser *p) {
    Token *op = previous(p);
    PExpr *cond = rExpression(p);
    eat(p, T_DO, "Expected do after while expression");
    PStmt *body = rToEndBlockStmt(p);
    return NewWhileStmt(p->gc, op, cond, body);
}

static PStmt *rReturnStmt(Parser *p) {
    Token *op = previous(p);
    PExpr *value = NULL;

    if (!check(p, T_SEMICOLON)) {
        value = rExpression(p);
    }

    if (check(p, T_SEMICOLON)) {
        eat(p, T_SEMICOLON, "Expected ';' after empty return");
    }
    return NewReturnStmt(p->gc, op, value);
}

static PStmt *rBreakStmt(Parser *p) {
    Token *op = previous(p);
    if (check(p, T_SEMICOLON)) {
        eat(p, T_SEMICOLON, "Expected ';'");
    }

    return NewBreakStmt(p->gc, op);
}

static PStmt *rFuncStmt(Parser *p) {
    Token *name = eat(p, T_IDENT, "Expected function name");
    eat(p, T_LEFT_PAREN, "Expected '(' after function name");
    Token **params = NULL;
    int paramCount = 0;
    if (!check(p, T_RIGHT_PAREN)) {
        do {
            Token *parm = eat(p, T_IDENT, "Expected param");
            arrput(params, parm);
            paramCount++;
        } while (matchOne(p, T_COMMA));
    }
    eat(p, T_RIGHT_PAREN, "Expected ')' after expression list");
    PStmt *body = rToEndBlockStmt(p);

    return NewFuncStmt(p->gc, name, params, body, paramCount);
}

static PStmt *rStmt(Parser *p) {
    if (matchOne(p, T_PRINT)) {
        return rPrintStmt(p);
    } else if (matchOne(p, T_LEFT_BRACE)) {
        return rBlockStmt(p);
    } else if (matchOne(p, T_IF)) {
        return rIfStmt(p);
    } else if (matchOne(p, T_WHILE)) {
        return rWhileStmt(p);
    } else if (matchOne(p, T_RETURN)) {
        return rReturnStmt(p);
    } else if (matchOne(p, T_BREAK)) {
        return rBreakStmt(p);
    } else if (matchOne(p, T_FUNC)) {
        return rFuncStmt(p);
    }
    return rExprStmt(p);
}

static PStmt *rLet(Parser *p) {
    if (matchOne(p, T_LET)) {
        return rLetStmt(p);
    }

    PStmt *s = rStmt(p);
    if (p->hasError) {
        syncParser(p);
    }

    return s;
}
