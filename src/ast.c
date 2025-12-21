#include "include/ast.h"
#include "external/stb/stb_ds.h"
#include "include/ansicolors.h"
#include "include/printer.h"
#include "include/ptypes.h"
#include "include/token.h"
#include <stddef.h>
#include <stdio.h>

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

    PanPrint(
        TERMC_YELLOW "%s(" TERMC_RESET "%s" TERMC_YELLOW ")" TERMC_RESET,
        LiteralTypeToStr(lit->type), lit->op->lexeme
    );
    PanPrint("\n");
}

static void printIndent(int indent) {
    for (int i = 0; i < indent; i++) {
        PanPrint("  ");
    }
}

void AstPrint(PExpr *expr, int indent) {
    printIndent(indent);
    if (expr == NULL) {
        PanPrint("[Invalid Expression!]\n");
        return;
    }
    switch (expr->type) {
        case EXPR_BINARY: {
            PanPrint(
                TERMC_PURPLE "BinaryExpr(" TERMC_RESET "%s" TERMC_PURPLE
                             ")\n" TERMC_RESET,
                TokTypeToStr(expr->exp.EBinary.op->type)
            );

            AstPrint(expr->exp.EBinary.left, indent + 1);
            AstPrint(expr->exp.EBinary.right, indent + 1);
            break;
        }
        case EXPR_UNARY: {
            PanPrint(
                TERMC_GREEN "Unary(" TERMC_RESET "%s" TERMC_GREEN
                            ")\n" TERMC_RESET,
                TokTypeToStr(expr->exp.EBinary.op->type)
            );

            AstPrint(expr->exp.EUnary.right, indent + 1);
            break;
        }
        case EXPR_LITERAL: {
            AstPrintLiteral(expr);
            break;
        }
        case EXPR_GROUPING: {
            PanPrint(TERMC_GREEN "Group\n" TERMC_RESET);

            AstPrint(expr->exp.EGrouping.expr, indent + 1);
            break;
        }
        case EXPR_VARIABLE: {
            PanPrint(
                TERMC_RED "Var(" TERMC_GREEN "%s" TERMC_RED ")\n" TERMC_RESET,
                expr->exp.EVariable.name->lexeme
            );

            break;
        }
        case EXPR_ASSIGN: {
            PanPrint(TERMC_YELLOW "Assign {\n" TERMC_RESET);
            printIndent(indent + 1);
            PanPrint("Target {\n");
            AstPrint(expr->exp.EAssign.name, indent + 2);
            printIndent(indent + 1);
            PanPrint("}\n");

            printIndent(indent + 1);
            PanPrint("Value {\n");
            AstPrint(expr->exp.EAssign.value, indent + 2);
            printIndent(indent + 1);
            PanPrint("}\n");
            printIndent(indent);
            PanPrint("}\n");
            break;
        }
        case EXPR_LOGICAL: {
            PanPrint("Logical : {}\n");
            break;
        }
        case EXPR_CALL: {
            struct ECall *call = &expr->exp.ECall;
            PanPrint(TERMC_YELLOW "Call" TERMC_RESET);
            PanPrint("{\n");
            printIndent(indent + 1);
            PanPrint("Callee {\n");
            AstPrint(call->callee, indent + 2);
            printIndent(indent + 1);
            PanPrint("}\n");

            printIndent(indent + 1);
            PanPrint("Args (%zu) {\n", call->argCount);
            for (u64 i = 0; i < call->argCount; i++) {
                AstPrint(call->args[i], indent + 2);
            }
            printIndent(indent + 1);
            PanPrint("}\n");

            printIndent(indent);
            PanPrint("}\n");
            break;
        }
        case EXPR_ARRAY: {
            struct EArray *arr = &expr->exp.EArray;
            PanPrint(
                TERMC_RED "Array (" TERMC_GREEN "%zu" TERMC_RED
                          ") " TERMC_RESET,
                arr->count
            );
            PanPrint("{\n");
            for (u64 i = 0; i < arr->count; i++) {
                AstPrint(arr->items[i], indent + 2);
            }
            printIndent(indent);
            PanPrint("}\n");
            break;
        }
        case EXPR_MAP: {
            struct EMap *map = &expr->exp.EMap;
            PanPrint(
                TERMC_RED "Map (" TERMC_GREEN "%zu" TERMC_RED ") " TERMC_RESET,
                map->count / 2
            );
            PanPrint("{\n");
            for (u64 i = 0; i < map->count; i += 2) {
                printIndent(indent + 1);
                PanPrint("{\n");
                AstPrint(map->etable[i], indent + 2);

                AstPrint(map->etable[i + 1], indent + 2);
                printIndent(indent + 1);
                PanPrint("}\n");
            }
            printIndent(indent);
            PanPrint("}\n");
            break;
        }
        case EXPR_SUBSCRIPT: {
            struct ESubscript *sub = &expr->exp.ESubscript;
            PanPrint(TERMC_YELLOW "Subscript " TERMC_RESET);
            PanPrint("{\n");
            printIndent(indent + 1);
            PanPrint("Value {\n");
            AstPrint(sub->value, indent + 2);
            printIndent(indent + 1);
            PanPrint("}\n");
            printIndent(indent + 1);
            PanPrint("Index {\n");
            AstPrint(sub->index, indent + 2);
            printIndent(indent + 1);
            PanPrint("}\n");
            printIndent(indent);
            PanPrint("}\n");
            break;
        }

        case EXPR_MODGET: {
            struct EModget *mg = &expr->exp.EModget;
            PanPrint(TERMC_YELLOW "ModuleGet" TERMC_RESET);
            PanPrint("{\n");
            printIndent(indent + 1);
            PanPrint("Module {\n");
            AstPrint(mg->module, indent + 2);
            printIndent(indent + 1);
            PanPrint("}\n");
            printIndent(indent + 1);
            PanPrint(
                "Child(" TERMC_GREEN "%s" TERMC_RESET ")\n", mg->child->lexeme
            );
            printIndent(indent);
            PanPrint("}\n");
            break;
        }
    }
}

void AstStmtPrint(PStmt *stmt, int indent) {
    if (stmt == NULL) {
        PanPrint("Invalid Statement\n");
        return;
    }
    printIndent(indent);
    switch (stmt->type) {
        case STMT_EXPR: {
            PanPrint("Expr [\n");
            AstPrint(stmt->stmt.SExpr.expr, indent + 1);
            printIndent(indent);
            PanPrint("]\n");
            break;
        }
        case STMT_LET: {
            PanPrint(
                "Let (" TERMC_GREEN "%s" TERMC_RESET ") [\n",
                stmt->stmt.SLet.name->lexeme
            );

            AstPrint(stmt->stmt.SLet.expr, indent + 1);
            printIndent(indent);
            PanPrint("]\n");
            break;
        }
        case STMT_BLOCK: {
            PanPrint("Block [\n");
            for (long i = 0; i < arrlen(stmt->stmt.SBlock.stmts); i++) {
                AstStmtPrint(stmt->stmt.SBlock.stmts[i], indent + 1);
            }
            printIndent(indent);
            PanPrint("]\n");

            break;
        }
        case STMT_IF: {
            PanPrint("If [\n");
            printIndent(indent + 1);
            PanPrint("Cond {\n");
            AstPrint(stmt->stmt.SIf.cond, indent + 2);
            printIndent(indent + 1);
            PanPrint("}\n");
            printIndent(indent + 1);
            PanPrint("Then {\n");
            AstStmtPrint(stmt->stmt.SIf.thenBranch, indent + 2);
            printIndent(indent + 1);
            PanPrint("}\n");
            printIndent(indent + 1);
            PanPrint("Else {\n");
            if (stmt->stmt.SIf.elseBranch != NULL) {
                AstStmtPrint(stmt->stmt.SIf.elseBranch, indent + 2);
            }
            printIndent(indent + 1);
            PanPrint("}\n");
            printIndent(indent);
            PanPrint("]\n");

            break;
        }
        case STMT_WHILE: {
            PanPrint("While [\n");
            printIndent(indent + 1);
            PanPrint("Cond {\n");
            AstPrint(stmt->stmt.SWhile.cond, indent + 2);
            printIndent(indent + 1);
            PanPrint("}\n");
            printIndent(indent + 1);
            PanPrint("Body {\n");
            AstStmtPrint(stmt->stmt.SWhile.body, indent + 2);
            printIndent(indent + 1);
            PanPrint("}\n");
            printIndent(indent);
            PanPrint("]\n");
            break;
        }

        case STMT_RETURN: {
            PanPrint("Return [\n");
            AstPrint(stmt->stmt.SReturn.value, indent + 1);
            printIndent(indent);
            PanPrint("]\n");
            break;
        }
        case STMT_BREAK: {
            PanPrint("Break []\n");
            break;
        }
        case STMT_FUNC: {
            struct SFunc *fn = &stmt->stmt.SFunc;

            PanPrint(
                "Func(" TERMC_GREEN "%s" TERMC_RESET ") <", fn->name->lexeme
            );
            PanPrint(TERMC_GREEN);
            for (u64 i = 0; i < fn->paramCount; i++) {
                PanPrint("%s", fn->params[i]->lexeme);
                if (i != fn->paramCount - 1) {
                    PanPrint(TERMC_RESET ", " TERMC_GREEN);
                }
            }
            PanPrint(TERMC_RESET "> [\n");
            AstStmtPrint(fn->body, indent + 1);
            printIndent(indent);
            PanPrint("]\n");
            break;
        }
        case STMT_IMPORT: {
            struct SImport *import = &stmt->stmt.SImport;
            PanPrint(
                "Import(" TERMC_GREEN "%s" TERMC_RESET ") ",
                import->name->lexeme
            );
            PanPrint("{\n");
            AstPrint(import->path, indent + 2);
            printIndent(indent);
            PanPrint("}\n");
            break;
        }
    }
}

char *StmtTypeToStr(PStmtType type) {
    switch (type) {
        case STMT_RETURN: return "Return Stmt";
        case STMT_WHILE: return "While Stmt";
        case STMT_IF: return "If Stmt";
        case STMT_BLOCK: return "Block Stmt";
        case STMT_LET: return "Let Stmt";
        case STMT_EXPR: return "Expr Stmt";
        case STMT_BREAK: return "Break Stmt";
        case STMT_FUNC: return "Func Stmt";
        case STMT_IMPORT: return "Import Stmt";
    }

    return "Unknown Statement";
}
