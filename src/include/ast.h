#ifndef AST_H
#define AST_H
#include "token.h"

typedef enum PExprType{
	EXPR_BINARY,
	EXPR_UNARY,
	EXPR_LITERAL,
	EXPR_GROUPING,
	EXPR_CONDITION
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

	}exp;
}PExpr;



#define ToExpr(e) (PExpr*)e
PExpr * NewBinaryExpr(PExpr * left, Token * op, PExpr * right);
PExpr * NewUnary(Token * op, PExpr * right);
PExpr * NewLiteral(Token * op, ExpLitType type);
PExpr * NewGrouping(PExpr * expr);



void AstPrint(PExpr * expr);

#endif
