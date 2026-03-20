#include "include/ast.h"
#include "external/stb/stb_ds.h"
#include "include/printer.h"
#include "include/ptypes.h"
#include "include/terminal.h"
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

    PanFPrint(
        stdout, "%s%s(%s%s%s)%s", TermYellow(), LiteralTypeToStr(lit->type),
        TermReset(), lit->op->lexeme, TermYellow(), TermReset()
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
        PanPrint("%s[Invalid Expression!]%s\n", TermRed(), TermReset());
        return;
    }
    switch (expr->type) {
        case EXPR_BINARY: {
            PanPrint(
                "%sBinaryExpr(%s"
                "%s"
                "%s)%s\n",
                TermPurple(), TermReset(),
                TokTypeToStr(expr->exp.EBinary.op->type), TermPurple(),
                TermReset()
            );

            AstPrint(expr->exp.EBinary.left, indent + 1);
            AstPrint(expr->exp.EBinary.right, indent + 1);
            break;
        }
        case EXPR_UNARY: {
            PanPrint(
                "%sUnary(%s"
                "%s"
                "%s)%s\n",
                TermGreen(), TermReset(),
                TokTypeToStr(expr->exp.EUnary.op->type), TermGreen(),
                TermReset()

            );

            AstPrint(expr->exp.EUnary.right, indent + 1);
            break;
        }
        case EXPR_LITERAL: {
            AstPrintLiteral(expr);
            break;
        }
        case EXPR_GROUPING: {
            PanPrint("%sGroup%s\n", TermGreen(), TermReset());

            AstPrint(expr->exp.EGrouping.expr, indent + 1);
            break;
        }
        case EXPR_VARIABLE: {
            PanPrint(
                "%sVar(%s"
                "%s"
                "%s)%s\n",
                TermRed(), TermGreen(), expr->exp.EVariable.name->lexeme,
                TermRed(), TermReset()
            );

            break;
        }
        case EXPR_ASSIGN: {
            PanPrint("%sAssign%s {\n", TermYellow(), TermReset());
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
            PanPrint("%sCall%s", TermYellow(), TermReset());
            PanPrint("{\n");
            printIndent(indent + 1);
            PanPrint("Callee {\n");
            AstPrint(call->callee, indent + 2);
            printIndent(indent + 1);
            PanPrint("}\n");

            printIndent(indent + 1);
            PanPrint("Args (%llu) {\n", (unsigned long long)call->argCount);
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
                "%sArray ("
                "%s%llu"
                "%s)%s ",
                TermRed(), TermGreen(), (unsigned long long)arr->count,
                TermRed(), TermReset()
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
                "%sMap ("
                "%s%llu"
                "%s)%s ",
                TermRed(), TermGreen(), (unsigned long long)map->count / 2,
                TermRed(), TermReset()
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
            PanPrint("%sSubscript%s ", TermYellow(), TermReset());
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
            PanPrint("%sModuleGet%s", TermYellow(), TermReset());
            PanPrint("{\n");
            printIndent(indent + 1);
            PanPrint("Module {\n");
            AstPrint(mg->module, indent + 2);
            printIndent(indent + 1);
            PanPrint("}\n");
            printIndent(indent + 1);
            char *childName = NULL;
            if (mg->child != NULL) {
                childName = mg->child->lexeme;
            }
            PanPrint(
                "Child("
                "%s%s%s)\n",
                TermGreen(), childName != NULL ? childName : "<error>",
                TermReset()
            );
            printIndent(indent);
            PanPrint("}\n");
            break;
        }
    }
}

void AstStmtPrint(PStmt *stmt, int indent) {
    if (stmt == NULL) {
        PanPrint("%s[Invalid Statement]%s\n", TermRed(), TermReset());
        return;
    }
    printIndent(indent);
    switch (stmt->type) {
        case STMT_DEBUG: {
            PanPrint("DebugPrint [\n");
            AstPrint(stmt->stmt.SDebug.expr, indent + 1);
            printIndent(indent);
            PanPrint("]\n");
            break;
        }
        case STMT_EXPR: {
            PanPrint("Expr [\n");
            AstPrint(stmt->stmt.SExpr.expr, indent + 1);
            printIndent(indent);
            PanPrint("]\n");
            break;
        }
        case STMT_LET: {
            PanPrint(
                "Let ("
                "%s%s%s"
                ") [\n",
                TermGreen(), stmt->stmt.SLet.name->lexeme, TermReset()
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
        case STMT_CONTINUE: {
            PanPrint("Continue []\n");
            break;
        }
        case STMT_FUNC: {
            struct SFunc *fn = &stmt->stmt.SFunc;

            PanPrint(
                "Func("
                "%s%s%s"
                ") <",
                TermGreen(), fn->name->lexeme, TermReset()
            );
            PanPrint(TermGreen());
            for (u64 i = 0; i < fn->paramCount; i++) {
                PanPrint("%s", fn->params[i]->lexeme);
                if (i != fn->paramCount - 1) {
                    PanPrint("%s, %s", TermReset(), TermGreen());
                }
            }
            PanPrint("%s> [\n", TermReset());
            AstStmtPrint(fn->body, indent + 1);
            printIndent(indent);
            PanPrint("]\n");
            break;
        }
        case STMT_IMPORT: {
            struct SImport *import = &stmt->stmt.SImport;
            PanPrint(
                "Import("
                "%s%s%s"
                ") ",
                TermGreen(), import->name->lexeme, TermReset()
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
        case STMT_CONTINUE: return "Continue Stmt";
        case STMT_FUNC: return "Func Stmt";
        case STMT_IMPORT: return "Import Stmt";
        case STMT_DEBUG: return "Debug Stmt";
    }

    return "Unknown Statement";
}
