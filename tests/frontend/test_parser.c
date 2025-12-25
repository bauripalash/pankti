#include "../../src/include/lexer.h"
#include "../../src/include/parser.h"
#include "../../src/include/gc.h"
#include "../../src/external/stb/stb_ds.h"
#include "../include/utest.h"

struct ParserTest{
	Lexer * lx;
	Parser * parser;
	Pgc * gc;
	PStmt ** stmts;
};

UTEST_F_SETUP(ParserTest){
	utest_fixture->lx = NULL;
	utest_fixture->parser = NULL;
	utest_fixture->gc = NULL;
	utest_fixture->stmts = NULL;
}

UTEST_F_TEARDOWN(ParserTest){
	if (utest_fixture->parser != NULL) {
		FreeParser(utest_fixture->parser);
	}

	if (utest_fixture->lx != NULL) {
		FreeLexer(utest_fixture->lx);
	}

	if (utest_fixture->gc != NULL) {
		FreeGc(utest_fixture->gc);
	}
}

#define SetupParser(src)\
	utest_fixture->gc = NewGc();\
	utest_fixture->lx = NewLexer(src);\
    MakeLexerRaw(utest_fixture->lx, true);\
	ScanTokens(utest_fixture->lx);\
	utest_fixture->parser = NewParser(utest_fixture->gc, utest_fixture->lx);\
	utest_fixture->stmts = ParseParser(utest_fixture->parser);\


UTEST_F(ParserTest, OperatorPrecAddMul){
	SetupParser("1+2*3");
	// This should translate to (+ 1 (* 2 3))
	ASSERT_EQ(arrlen(utest_fixture->stmts), 1);
	PStmt * stmt = utest_fixture->stmts[0];

	ASSERT_EQ(stmt->type, STMT_EXPR);
	PExpr * expr = stmt->stmt.SExpr.expr;
	ASSERT_EQ(expr->type, EXPR_BINARY);
	ASSERT_EQ(expr->op->type, T_PLUS);

	// Left : 1 
	ASSERT_EQ(expr->exp.EBinary.left->type, EXPR_LITERAL);
	ASSERT_EQ(expr->exp.EBinary.left->exp.ELiteral.type, EXP_LIT_NUM);
	ASSERT_EQ(expr->exp.EBinary.left->exp.ELiteral.value.nvalue, 1.0);

	// Right (2 * 3)
	PExpr * right = expr->exp.EBinary.right;
	ASSERT_EQ(right->type, EXPR_BINARY);
	ASSERT_EQ(right->op->type, T_ASTR);
	ASSERT_EQ(right->exp.EBinary.left->exp.ELiteral.value.nvalue, 2.0);
	ASSERT_EQ(right->exp.EBinary.right->exp.ELiteral.value.nvalue, 3.0);

}

UTEST_F(ParserTest, OperatorPrecDivPow){
	SetupParser("1/2**3");
	// This should translate to (/ 1 (** 2 3))
	ASSERT_EQ(arrlen(utest_fixture->stmts), 1);
	PStmt * stmt = utest_fixture->stmts[0];

	ASSERT_EQ(stmt->type, STMT_EXPR);
	PExpr * expr = stmt->stmt.SExpr.expr;
	ASSERT_EQ(expr->type, EXPR_BINARY);
	ASSERT_EQ(expr->op->type, T_SLASH);

	// Left : 1 
	ASSERT_EQ(expr->exp.EBinary.left->type, EXPR_LITERAL);
	ASSERT_EQ(expr->exp.EBinary.left->exp.ELiteral.type, EXP_LIT_NUM);
	ASSERT_EQ(expr->exp.EBinary.left->exp.ELiteral.value.nvalue, 1.0);

	// Right (2 ** 3)
	PExpr * right = expr->exp.EBinary.right;
	ASSERT_EQ(right->type, EXPR_BINARY);
	ASSERT_EQ(right->op->type, T_EXPONENT);
	ASSERT_EQ(right->exp.EBinary.left->exp.ELiteral.value.nvalue, 2.0);
	ASSERT_EQ(right->exp.EBinary.right->exp.ELiteral.value.nvalue, 3.0);
}

UTEST_F(ParserTest, Let){
	SetupParser("dhori time = 0");
	ASSERT_EQ(arrlen(utest_fixture->stmts), 1);
	PStmt * stmt = utest_fixture->stmts[0];

	ASSERT_EQ(stmt->type, STMT_LET);
	struct SLet * let = &stmt->stmt.SLet;
	ASSERT_STREQ(let->name->lexeme, "time");
	ASSERT_EQ(let->expr->type, EXPR_LITERAL);
}

UTEST_F(ParserTest, FuncDeclaration){
	SetupParser("kaj jog(a, b) ferao a + b sesh");
	ASSERT_EQ(arrlen(utest_fixture->stmts), 1);
	PStmt * stmt = utest_fixture->stmts[0];

	ASSERT_EQ(stmt->type, STMT_FUNC);
	ASSERT_STREQ(stmt->stmt.SFunc.name->lexeme, "jog");

	ASSERT_EQ(stmt->stmt.SFunc.paramCount, 2);
	ASSERT_STREQ(stmt->stmt.SFunc.params[0]->lexeme, "a");
	ASSERT_STREQ(stmt->stmt.SFunc.params[1]->lexeme, "b");

	PStmt * fbody = stmt->stmt.SFunc.body;
	ASSERT_EQ(fbody->type, STMT_BLOCK);

	struct SBlock * block = &fbody->stmt.SBlock;
	ASSERT_EQ(arrlen(block->stmts), 1);
	ASSERT_EQ(block->stmts[0]->type, STMT_RETURN);

	struct SReturn * ret = &block->stmts[0]->stmt.SReturn;
	ASSERT_EQ(ret->value->type, EXPR_BINARY);
}
