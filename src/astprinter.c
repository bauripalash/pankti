#include "external/stb/stb_ds.h"
#include "include/ansicolors.h"
#include "include/ast.h"
#include "include/token.h"
#include <stdio.h>

static void printIndent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

static char *LiteralTypeToStr(ExpLitType type) {
    switch (type) {
        case EXP_LIT_NUM: return "Number";
        case EXP_LIT_BOOL: return "Bool";
        case EXP_LIT_STR: return "String";
        case EXP_LIT_NIL: return "Nil";
    }

    return "Unknown";
}

void AstPrintLiteral(PExpr *expr) {
    if (expr == NULL) {
        return;
    }

    if (expr->type != EXPR_LITERAL) {
        return;
    }

    struct ELiteral *lit = &expr->exp.ELiteral;

    printf(
        TERMC_YELLOW "%s(" TERMC_RESET "%s" TERMC_YELLOW ")" TERMC_RESET,
        LiteralTypeToStr(lit->type), lit->op->lexeme
    );
    printf("\n");
}

void AstPrint(PExpr *expr, int indent) {
    if (expr == NULL) {
        printf("Invalid Expression!\n");
        return;
    }

    printIndent(indent);

    switch (expr->type) {
        case EXPR_BINARY: {
            printf(
                TERMC_PURPLE "BinaryExpr(" TERMC_RESET "%s" TERMC_PURPLE
                             ")\n" TERMC_RESET,
                TokTypeToStr(expr->exp.EBinary.opr->type)
            );

            AstPrint(expr->exp.EBinary.left, indent + 1);
            AstPrint(expr->exp.EBinary.right, indent + 1);
            break;
        }
        case EXPR_UNARY: {
            printf(
                TERMC_GREEN "Unary(" TERMC_RESET "%s" TERMC_GREEN
                            ")\n" TERMC_RESET,
                TokTypeToStr(expr->exp.EBinary.opr->type)
            );

            AstPrint(expr->exp.EUnary.right, indent + 1);
            break;
        }
        case EXPR_LITERAL: {
            AstPrintLiteral(expr);
            break;
        }
        case EXPR_GROUPING: {
            printf(TERMC_GREEN "Group\n" TERMC_RESET);

            AstPrint(expr->exp.EGrouping.expr, indent + 1);
            break;
        }
        case EXPR_VARIABLE: {
            printf(
                TERMC_RED "Var(" TERMC_GREEN "%s" TERMC_RED ")\n" TERMC_RESET,
                expr->exp.EVariable.name->lexeme
            );

            break;
        }
        case EXPR_ASSIGN: {
            printf(
                TERMC_YELLOW "Assign(" TERMC_GREEN "%s" TERMC_YELLOW
                             ")\n" TERMC_RESET,
                expr->exp.EAssign.name->exp.EVariable.name->lexeme
            );

            AstPrint(expr->exp.EAssign.value, indent + 1);
            break;
        }
        case EXPR_LOGICAL: {
            printf("Logical : {}\n");
            break;
        }
        case EXPR_CALL: {
            struct ECall *call = &expr->exp.ECall;
            printf(TERMC_YELLOW "Call" TERMC_RESET);
            printf("{\n");
            printIndent(indent + 1);
            printf("Callee {\n");
            AstPrint(call->callee, indent + 2);
            printIndent(indent + 1);
            printf("}\n");

            printIndent(indent + 1);
            printf("Args (%d) {\n", call->argCount);
            for (int i = 0; i < call->argCount; i++) {
                AstPrint(call->args[i], indent + 2);
            }
            printIndent(indent + 1);
            printf("}\n");

            printIndent(indent);
            printf("}\n");
            break;
        }
    }
}

void AstStmtPrint(PStmt *stmt, int indent) {
    if (stmt == NULL) {
        printf("Invalid Statement\n");
        return;
    }
    printIndent(indent);
    switch (stmt->type) {
        case STMT_PRINT: {
            printf("Print [\n");
            AstPrint(stmt->stmt.SPrint.value, indent + 1);
            printIndent(indent);
            printf("]\n");
            break;
        }

        case STMT_EXPR: {
            printf("Expr [\n");
            AstPrint(stmt->stmt.SExpr.expr, indent + 1);
            printf("\n]\n");
            break;
        }
        case STMT_LET: {
            printf(
                "Let (" TERMC_GREEN "%s" TERMC_RESET ") [\n",
                stmt->stmt.SLet.name->lexeme
            );

            AstPrint(stmt->stmt.SLet.expr, indent + 1);
            printf("\n]\n");
            break;
        }
        case STMT_BLOCK: {
            printf("Block [\n");
            for (int i = 0; i < arrlen(stmt->stmt.SBlock.stmts); i++) {
                AstStmtPrint(stmt->stmt.SBlock.stmts[i], indent + 1);
            }
            printIndent(indent);
            printf("]\n");

            break;
        }
        case STMT_IF: {
            printf("If [\n");
            printIndent(indent + 1);
            printf("Cond {\n");
            AstPrint(stmt->stmt.SIf.cond, indent + 2);
            printIndent(indent + 1);
            printf("}\n");
            printIndent(indent + 1);
            printf("Then {\n");
            AstStmtPrint(stmt->stmt.SIf.thenBranch, indent + 2);
            printIndent(indent + 1);
            printf("}\n");
            printIndent(indent + 1);
            printf("Else {\n");
            if (stmt->stmt.SIf.elseBranch != NULL) {
                AstStmtPrint(stmt->stmt.SIf.elseBranch, indent + 2);
            }
            printIndent(indent + 1);
            printf("}\n");
            printIndent(indent);
            printf("]\n");

            break;
        }
        case STMT_WHILE: {
            printf("While [\n");
            printIndent(indent + 1);
            printf("Cond {\n");
            AstPrint(stmt->stmt.SWhile.cond, indent + 2);
            printIndent(indent + 1);
            printf("}\n");
            printIndent(indent + 1);
            printf("Body {\n");
            AstStmtPrint(stmt->stmt.SWhile.body, indent + 2);
            printIndent(indent + 1);
            printf("}\n");
            printIndent(indent);
            printf("]\n");
            break;
        }

        case STMT_RETURN: {
            printf("Return [\n");
            AstPrint(stmt->stmt.SReturn.value, indent + 1);
            printIndent(indent);
            printf("]\n");
            break;
        }
        case STMT_BREAK: {
            printf("Break []\n");
            break;
        }
        case STMT_FUNC: {
            struct SFunc *fn = &stmt->stmt.SFunc;

            printf(
                "Func(" TERMC_GREEN "%s" TERMC_RESET ") <", fn->name->lexeme
            );
            printf(TERMC_GREEN);
            for (int i = 0; i < fn->paramCount; i++) {
                printf("%s", fn->params[i]->lexeme);
                if (i != fn->paramCount - 1) {
                    printf(TERMC_RESET ", " TERMC_GREEN);
                }
            }
            printf(TERMC_RESET "> [\n");
            AstStmtPrint(fn->body, indent + 1);
            printIndent(indent);
            printf("]\n");
            break;
        }
    }
}

char *StmtTypeToStr(PStmtType type) {
    switch (type) {
        case STMT_RETURN: return "Return Stmt"; break;
        case STMT_WHILE: return "While Stmt"; break;
        case STMT_IF: return "If Stmt"; break;
        case STMT_BLOCK: return "Block Stmt"; break;
        case STMT_LET: return "Let Stmt"; break;
        case STMT_EXPR: return "Expr Stmt"; break;
        case STMT_PRINT: return "Print Stmt"; break;
        case STMT_BREAK: return "Break Stmt"; break;
        case STMT_FUNC: return "Func Stmt"; break;
    }

    return "Unknown Statement";
}
