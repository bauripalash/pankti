#ifndef AST_H
#define AST_H
#include "token.h"

typedef enum PExprType{
	EXPR_BINARY,
	EXPR_UNARY,
	EXPR_LITERAL,
	EXPR_GROUPING,
	EXPR_VARIABLE,
	EXPR_ASSIGN,
}PExprType;

typedef enum PLitType{
	EXP_LIT_BOOL,
	EXP_LIT_NUM,
	EXP_LIT_NIL,
	EXP_LIT_STR,
}ExpLitType;

typedef struct PExpr{
	PExprType type;
	union exp{
		struct EBinary{
			struct PExpr * left;
			Token * opr;
			struct PExpr * right;
		}EBinary;

		struct EUnary{
			struct PExpr * right;
			Token * opr;
		}EUnary;

		struct ELiteral{
			Token * op;
			ExpLitType type;
			union value{
				bool bvalue;
				double nvalue;
			}value;
		}ELiteral;

		struct EGrouping{
			struct PExpr * expr;
		}EGrouping;

		struct EVariable{
			Token * name;
		}EVariable;

		struct EAssign{
			Token * name;
			struct PExpr * value;
		}EAssign;

	}exp;
}PExpr;

typedef enum PStmtType{
	STMT_EXPR = 1,
	STMT_PRINT,
	STMT_IF ,
	STMT_LET,
	STMT_FUNC,
	STMT_WHILE,
	STMT_BLOCK,
}PStmtType;

typedef struct PStmt{
	PStmtType type;
	union stmt{
		struct SPrint{
			Token * op;
			PExpr * value;
		}SPrint;

		struct SExpr{
			Token * op;
			PExpr * expr;
		}SExpr;

		struct SLet{
			Token * name;
			PExpr * expr;
		}SLet;

		struct SBlock{
			Token * op;
			struct PStmt ** stmts;
		}SBlock;
	}stmt;

}PStmt;


#define ToExpr(e) (PExpr*)e
PExpr * NewExpr(PExprType type);
PExpr * NewBinaryExpr(PExpr * left, Token * op, PExpr * right);
PExpr * NewUnary(Token * op, PExpr * right);
PExpr * NewLiteral(Token * op, ExpLitType type);
PExpr * NewGrouping(PExpr * expr);
PExpr * NewVarExpr(Token * name);
PExpr * NewAssignment(Token * name, PExpr * value);

PStmt * NewStmt(PStmtType type);
PStmt * NewPrintStmt(Token * op, PExpr * value);
PStmt * NewExprStmt(Token * op, PExpr * value);
PStmt * NewLetStmt(Token * name, PExpr * value);
PStmt * NewBlockStmt(Token * op, PStmt ** stmts);



void AstPrint(PExpr * expr);
void AstStmtPrint(PStmt * stmt);

#endif
