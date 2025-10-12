#include "include/parser.h"
#include "include/alloc.h"
#include "include/ast.h"
#include "include/core.h"
#include "include/lexer.h"
#include "external/stb/stb_ds.h"
#include "include/token.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static PStmt * rLet(Parser * p);
static PStmt * rStmt(Parser * p);


static PExpr * rAssignment(Parser * p);
static PExpr * rEquality(Parser * p);
static PExpr * rComparison(Parser * p);
static PExpr * rTerm(Parser * p);
static PExpr * rFactor(Parser * p);
static PExpr * rUnary(Parser * p);
static PExpr * rPrimary(Parser * p);
static PExpr * rExpression(Parser * p);

static bool atEnd(const Parser * p);

Parser * NewParser(Lexer * lexer){
	Parser * parser = PCreate(Parser);
	//ERROR HANDLE
	parser->lx = lexer;
	parser->tokens = lexer->tokens;
	parser->pos = 0;
	parser->core = NULL;
	parser->stmts = NULL;
	parser->hasError = false;
	return parser;
}
void FreeParser(Parser * parser){
	if (parser == NULL) {
		return;
	}
	free(parser);
}

PStmt ** ParseParser(Parser * parser){
	//return rExpression(parser);
	while (!atEnd(parser)) {
		arrput(parser->stmts, rLet(parser));
	}

	return parser->stmts;
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
	p->hasError = true;
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
	return rAssignment(p);
}

static PExpr * rAnd(Parser * p){
	PExpr * expr = rEquality(p);
	while (matchOne(p, T_AND)) {
		Token * op = previous(p);
		PExpr * right = rEquality(p);
		expr = NewLogical(expr, op, right);
	}
	return expr;
}

static PExpr * rOr(Parser * p){
	PExpr * expr = rAnd(p);
	while (matchOne(p, T_OR)) {
		Token * op = previous(p);
		PExpr * right = rAnd(p);
		expr = NewLogical(expr, op, right);
		
	}
	return expr;

}

static PExpr * rAssignment(Parser * p){
	PExpr * expr = rOr(p);
	if (matchOne(p, T_EQ)) {
		Token * op = previous(p);
		PExpr * value = rAssignment(p);

		if (expr->type == EXPR_VARIABLE) {
			Token * name = expr->exp.EVariable.name;
			return NewAssignment(name, value);
		}

		error(p, NULL, "Invalid assignment");
		return NULL;
	}

	return expr;

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
		return e;
	} 

	if (matchOne(p, T_FALSE)) {
		PExpr * e = NewLiteral(previous(p), EXP_LIT_BOOL);
		e->exp.ELiteral.value.bvalue = false;
		return e;
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

	if (matchOne(p, T_IDENT)) {
		return NewVarExpr(previous(p));
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

static PStmt * rExprStmt(Parser * p){
	PExpr * value = rExpression(p);
	return NewExprStmt(previous(p), value);
}

static PStmt * rPrintStmt(Parser * p){
	PExpr * value = rExpression(p);
	return NewPrintStmt(previous(p), value);
}

static PStmt * rLetStmt(Parser * p){
	Token * name = eat(p, T_IDENT, "Expected Identifier");
	eat(p, T_EQ, "Expected equal");
	PExpr * value = rExpression(p);
	return NewLetStmt(name, value);

}

static PStmt * rBlockStmt(Parser * p){
	PStmt ** stmtList = NULL;
	while (!check(p, T_RIGHT_BRACE) && !atEnd(p)) {
		arrput(stmtList, rLet(p));
	}

	eat(p, T_RIGHT_BRACE, "Expected '}' after block stmt");
	PStmt * block = NewBlockStmt(previous(p), stmtList);
	return block;
}

static PStmt * rToEndBlockStmt(Parser * p){
	PStmt ** stmtList = NULL;
	while (!check(p, T_END) && !atEnd(p)) {
		arrput(stmtList, rLet(p));
	}

	eat(p, T_END, "Expected 'end' after block");
	PStmt * block = NewBlockStmt(previous(p), stmtList);
	return block;
}

static PStmt * rToEndOrElseBlockStmt(Parser * p, bool * hasElse){
	PStmt ** stmtList = NULL;

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
	PStmt * block = NewBlockStmt(previous(p), stmtList);
	return block;
}
static PStmt * rIfStmt(Parser * p){
	Token * op = previous(p);
	PExpr * cond = rExpression(p);
	eat(p, T_THEN, "Expected then after if expression");
	bool hasElse = false;
	PStmt * thenBranch = rToEndOrElseBlockStmt(p, &hasElse);
	PStmt * elseBranch = NULL;
	if (hasElse) {
		elseBranch = rToEndBlockStmt(p);
	}

	return NewIfStmt(op, cond, thenBranch, elseBranch);
}

static PStmt * rWhileStmt(Parser * p){
	Token * op = previous(p);
	PExpr * cond = rExpression(p);
	eat(p, T_DO, "Expected do after while expression");
	PStmt * body = rToEndBlockStmt(p);
	return NewWhileStmt(op, cond, body);
}

static PStmt * rReturnStmt(Parser * p){
	Token * op = previous(p);
	PExpr * value = NULL;

	if (!check(p, T_SEMICOLON)) {
		value = rExpression(p);
		
	}

	if (check(p, T_SEMICOLON)) {
		eat(p, T_SEMICOLON, "Expected ';' after empty return");
	}
	return NewReturnStmt(op, value);

}


static PStmt * rStmt(Parser * p){
	if (matchOne(p, T_PRINT)) {
		return rPrintStmt(p);
	} else if (matchOne(p, T_LEFT_BRACE)){
		return rBlockStmt(p);
	} else if (matchOne(p, T_IF)) {
		return rIfStmt(p);
	} else if (matchOne(p, T_WHILE)){
		return rWhileStmt(p);
	} else if (matchOne(p, T_RETURN)) {
		return rReturnStmt(p);
	} 

	return rExprStmt(p);
}


static PStmt * rLet(Parser * p){
	if (matchOne(p, T_LET)) {
		return rLetStmt(p);
	}

	PStmt * s = rStmt(p);
	if (p->hasError) {
		syncParser(p);
	}

	return s;
}
