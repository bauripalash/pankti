#ifndef AST_H
#define AST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "token.h"

// Expression Type
typedef enum PExprType {
    // Binary Expression : a <Op> b ie. `100+200`
    EXPR_BINARY,
    // Unary Expression : <Op>a ie. `-100`
    EXPR_UNARY,
    // Literal Expression : Bool, Number, Nil and Strings.
    EXPR_LITERAL,
    // Grouping Expression : (<expression>) ie. `(1+2)`
    EXPR_GROUPING,
    // Variable Expression : Variable to resolve.
    // `myvar + 100` <myvar> in this case
    EXPR_VARIABLE,
    // Assignment Expression: set new value to preexisting variable.
    // `let a = 10`
    // `a = 10` <--
    EXPR_ASSIGN,
    // Logical Expression: or, and..
    EXPR_LOGICAL,
    // Calling expression:
    // call a variable which would be resolved to a function.
    // `myfunc(a,b,c)` <--
    EXPR_CALL,
} PExprType;

// Literal Types for Literal Expressions : `EXPR_LITERAL`
typedef enum PLitType {
    // Bool Literal
    EXP_LIT_BOOL,
    // Number Literal (always decimal; `double`)
    EXP_LIT_NUM,
    // Nil Literal
    EXP_LIT_NIL,
    // String Literal
    EXP_LIT_STR,
} ExpLitType;

// Pankti Expression Structure
typedef struct PExpr {
    // Expression type
    PExprType type;
    // Expression union of all type of expressions
    union exp {
        // Binary Operation Expression.
        // Type: `EXPR_BINARY`
        struct EBinary {
            // Left hand side expression
            struct PExpr *left;
            // Operator
            Token *opr;
            // Right  hand side expression
            struct PExpr *right;
        } EBinary;

        // Unary Operation Expression
        // Type: `EXPR_UNARY`
        struct EUnary {
            // Original expression
            struct PExpr *right;
            // Operator
            Token *opr;
        } EUnary;

        // Literal Expression; Bool, Number, String, Nil etc.
        // Type: `EXPR_LITERAL`
        // Bool and Numbers get directly evaluated
        struct ELiteral {
            // Raw Token
            Token *op;
            // Literal Type.
            ExpLitType type;
            union value {
                bool bvalue;
                double nvalue;
            } value;
        } ELiteral;

        // Grouping Expression
        // Type: `EXPR_GROUPING`
        struct EGrouping {
            // Original expression
            struct PExpr *expr;
        } EGrouping;

        // Variable Expression
        // Type: `EXPR_VARIABLE`
        // `myvar + 100` ; Here `myvar` in this case
        struct EVariable {
            // Raw token
            Token *name;
        } EVariable;

        // Assignment Expression
        // Type: `EXPR_ASSIGN`
        // Set a value to preexisting value
        struct EAssign {
            struct PExpr *name;
            struct PExpr *value;
        } EAssign;

        // Logical Expression
        // Type: `EXPR_LOGICAL`
        // Similar to Binary Expression but for logical operations, Or, And etc.
        struct ELogical {
            // Left hand side expression
            struct PExpr *left;
            Token *op;
            // Right hand side expression
            struct PExpr *right;
        } ELogical;

        // Calling Expression : Calling functions
        // Type: `EXPR_CALL`
        // `myfunction(a,b,c)`
        struct ECall {
            // Right parentheses of the call expression
            Token *op;
            // The Actual variable which should resolve to a function
            struct PExpr *callee;
            // Expression list of arguments
            struct PExpr **args;
            // Number of arguments provided
            int argCount;
        } ECall;
    } exp;
} PExpr;

// Statement Type
typedef enum PStmtType {
    // Expression Statement: Naked expressions
    STMT_EXPR = 1,
    // Print Statement : Temporary measure to print objects
    // NOTE: (should not be in final interpreter)
    STMT_PRINT,
    // If Statement
    STMT_IF,
    // Let Statement : Variable Declaration
    STMT_LET,
    // While Loop Statement
    STMT_WHILE,
    // Block Statement : Running statements inside a Temporary closure {...}
    STMT_BLOCK,
    // Return Statement : Used for functions
    STMT_RETURN,
    // Break Statement : Used only inside While Statements
    STMT_BREAK,
    // Function Declaration Statement
    STMT_FUNC,
} PStmtType;

// Pankti Statement
typedef struct PStmt {
    // Statement Type
    PStmtType type;
    struct PStmt *next;
    // `Temporary` Print Statement (for Debugging only)
    // Type: `STMT_PRINT`
    union stmt {
        struct SPrint {
            Token *op;
            PExpr *value;
        } SPrint;

        // Expression Statement : Naked Expressions
        // Type: `STMT_EXPR`
        struct SExpr {
            // Operator : Last Token in the Expression
            //  First token is not used because in an empty script with only
            //  one token (for example `1`) can result in a NULL token
            Token *op;
            // The expression
            PExpr *expr;
        } SExpr;

        // Let Statement : Variable Declaration
        // Type: `STMT_LET`
        struct SLet {
            // The variable name token
            Token *name;
            // The expression for the variable
            PExpr *expr;
        } SLet;

        // Block Statement : Used for closure, function, loops etc.
        // Type: `STMT_BLOCK`
        struct SBlock {
            // Left Brace `{`, the starting token of the block
            Token *op;
            // Statement array
            struct PStmt **stmts;
        } SBlock;

        // If Statement : Conditional if statement
        // Type: `STMT_IF`
        struct SIf {
            // The `IF` Token
            Token *op;
            // Conditional expression
            PExpr *cond;
            // Statement to execute if the `cond` expression is True
            // `thenBranch` will always be a Block Statement <`STMT_BLOCK`>
            struct PStmt *thenBranch;
            // Optional Statement to execute if the `cond` expression is False
            // `elseBranch` will always be a Block Statement <`STMT_BLOCK`>
            // elseBranch can be NULL
            struct PStmt *elseBranch;
        } SIf;

        // While Statement : Only Looping statement available for Pankti
        // Type: `STMT_WHILE`
        struct SWhile {
            // The `WHILE` token
            Token *op;
            // Conditional expression
            PExpr *cond;
            // Statement to execute to while the `cond` expression is True
            // `body` will always be a Block Statement <`STMT_BLOCK`>
            struct PStmt *body;
        } SWhile;

        // Return Statement : Used only in Functions
        // Type: `STMT_RETURN`
        // Does nothing if used anywhere else
        struct SReturn {
            // The `RETURN` token
            Token *op;
            // The actual expression which will be used
            PExpr *value;
        } SReturn;

        // Break Statement : Used only in while loop
        // Type: `STMT_BREAK`
        // Does nothing when used anywhere else
        struct SBreak {
            // The `BREAK` token
            Token *op;
        } SBreak;

        // Function Statement : Function Declaration
        // Type: `STMT_FUNC`
        struct SFunc {
            // The name of function
            Token *name;
            // Token array of parameters
            Token **params;
            // Number of parameters
            int paramCount;
            // The body. Will always be Block Statement <`STMT_BLOCK`>
            struct PStmt *body;
        } SFunc;
    } stmt;

} PStmt;

// Typecast `void *` to `PExpr *`
#define ToExpr(e) (PExpr *)e

// Print Expression AST
void AstPrint(PExpr *expr, int indent);
// Print Statement AST
void AstStmtPrint(PStmt *stmt, int indent);

// Get Statement Type as String (stack allocated)
char *StmtTypeToStr(PStmtType type);

#ifdef __cplusplus
}
#endif
#endif
