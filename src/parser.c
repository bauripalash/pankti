#include "include/parser.h"
#include "include/alloc.h"
#include "include/ast.h"
#include "include/core.h"
#include "include/lexer.h"
#include "external/stb/stb_ds.h"
#include "include/token.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static PExpr * rEquality(Parser * p);
static PExpr * rComparison(Parser * p);
static PExpr * rTerm(Parser * p);
static PExpr * rFactor(Parser * p);
static PExpr * rUnary(Parser * p);
static PExpr * rPrimary(Parser * p);
static PExpr * rExpression(Parser * p);

Parser * NewParser(Lexer * lexer){
	Parser * parser = PCreate(Parser);
	//ERROR HANDLE
	parser->lx = lexer;
	parser->tokens = lexer->tokens;
	parser->pos = 0;
	parser->core = NULL;
	return parser;
}
void FreeParser(Parser * parser){
	if (parser == NULL) {
		return;
	}
	free(parser);
}

PExpr * ParseParser(Parser * parser){
	return rExpression(parser);
}

static Token * peek(const Parser * p){
	return p->tokens[p->pos];
}

static bool atEnd(const Parser * p){
	return peek(p)->type == T_EOF;
}

static bool check(const Parser * p, TokenType t){

	if (atEnd(p)) {
		return false;
	}
	return peek(p)->type == t;
}

static Token * previous(const Parser * p){
	return p->tokens[p->pos - 1];
}

static Token * advance(Parser * p){
	if (!atEnd(p)) {
		p->pos++;
	}

	return previous(p);
}

static bool matchMany(Parser * p, TokenType * types, int count){
	for (int i = 0; i < count; i++) {
		if (check(p, types[i])) {
			advance(p);
			return true;
		}
	}

	return false;
}

static bool matchOne(Parser * p, TokenType type){
	if (check(p, type)) {
		advance(p);
		return true;
	}

	return false;
}

static void error(Parser * p, Token * tok, char * msg){
	CoreError(p->core, -1, msg);
}

static Token * eat(Parser * p, TokenType t, char * msg){
	if (check(p, t)) {
		return advance(p);
	}

	error(p, peek(p), msg);
	return NULL;
}

static PExpr * rExpression(Parser * p){
	return rEquality(p);
}

static PExpr * rEquality(Parser * p){
	PExpr * expr = rComparison(p);
	while (matchMany(p, (TokenType[]){T_BANG_EQ, T_EQEQ}, 2)) {
		Token * op = previous(p);
		PExpr * right = rComparison(p);
		expr = NewBinaryExpr(expr, op, right);

	}

	return expr;
}

static PExpr * rComparison(Parser * p){
	PExpr * expr = rTerm(p);
	while (matchMany(p, (TokenType[]){T_GT,T_GTE,T_LT,T_LTE}, 4)) {
		Token * op = previous(p);
		PExpr * right = rTerm(p);
		expr = NewBinaryExpr(expr, op, right);
	}

	return expr;
}

static PExpr * rTerm(Parser * p){
	PExpr * expr = rFactor(p);
	while (matchMany(p, (TokenType[]){T_MINUS, T_PLUS}, 2)) {
		Token * op = previous(p);
		PExpr * right = rFactor(p);
		expr = NewBinaryExpr(expr, op, right);
	}

	return expr;
}

static PExpr * rFactor(Parser * p){
	PExpr * expr = rUnary(p);
	while (matchMany(p, (TokenType[]){T_SLASH, T_ASTR}, 2)) {
		Token * op = previous(p);
		PExpr * right = rUnary(p);
		expr = NewBinaryExpr(expr, op, right);
	}
	return expr;
}

static PExpr * rUnary(Parser * p){
	if (matchMany(p, (TokenType[]){T_BANG, T_MINUS}, 2)) {
		Token * op = previous(p);
		PExpr * right = rUnary(p);
		return NewUnary(op, right);
	}

	return rPrimary(p);
}

static PExpr * rPrimary(Parser * p){
	if (matchOne(p, T_TRUE)) {
		PExpr * e = NewLiteral(previous(p), EXP_LIT_BOOL);
		e->exp.ELiteral.value.bvalue = true;
	} 

	if (matchOne(p, T_FALSE)) {
		PExpr * e = NewLiteral(previous(p), EXP_LIT_BOOL);
		e->exp.ELiteral.value.bvalue = false;
	}

	if (matchOne(p, T_NIL)) {
		return NewLiteral(previous(p), EXP_LIT_NIL);
	}

	if (matchOne(p, T_NUM)) {
		return NewLiteral(previous(p), EXP_LIT_NUM);
		//convert token value to number;
	}

	if (matchOne(p, T_STR)) {
		return NewLiteral(previous(p), EXP_LIT_STR);
	}

	if (matchOne(p, T_LEFT_PAREN)) {
		PExpr * e = rExpression(p);
		eat(p, T_RIGHT_PAREN, "Expected ')'");
		return NewGrouping(e);
	}

	error(p, previous(p), "Expected expression");
	return NULL;
}

static void syncParser(Parser * p){
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
			default:advance(p);break;
		}
	}
}
