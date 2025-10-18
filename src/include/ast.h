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
            Token *name;
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

// Create New (Empty) Expression
// Return => New Expression with type `type` or NULL in case of failure
PExpr *NewExpr(PExprType type);

// Free Expression
void FreeExpr(PExpr * expr);

// New Binary Expression. Type : `EXPR_BINARY`
// `op` = Raw operator token
// `left` = Left hand side expression
// `right` = Right hand side expression
// Return => New Binary Expression as Expression pointer or NULL
PExpr *NewBinaryExpr(PExpr *left, Token *op, PExpr *right);

// New Unary Expression. Type : `EXPR_UNARY`
// `op` = Raw operator token
// `right` = Right hand side [Original] expression
// Return => New Unary Expression as Expression pointer or NULL
PExpr *NewUnary(Token *op, PExpr *right);

// New Literal Expression (ie. Bool, String, Nil etc). Type : `EXPR_LITERAL`
// `op` = Raw token
// `type` = Literal type
// Return => New Literal Expression as Expression pointer or NULL
PExpr *NewLiteral(Token *op, ExpLitType type);

// New Grouping Expression. Type : `EXPR_GROUPING`
// `(<expr>)`
// Return => New Grouping Expression as Expression pointer or NULL
PExpr *NewGrouping(PExpr *expr);

// New Variable Expression. Type : `EXPR_VARIABLE`
// `name` = Variable name
// Return => New Variable Expression as Expression pointer or NULL
PExpr *NewVarExpr(Token *name);

// New Assignment Expression. Type : `EXPR_ASSIGN`
// Set `value` to a preexisting variable with name being `name`
// Return => New Assignment Expression as Expression pointer or NULL
PExpr *NewAssignment(Token *name, PExpr *value);

// New Logical Expression (ie. And, Or). Type : `EXPR_LOGICAL`
// Return => New Logical Expression as Expression pointer or NULL
PExpr *NewLogical(PExpr *left, Token *op, PExpr *right);

// New (Function) Calling Expression. Type : `EXPR_CALL`
// `op` = Right parentheses of the call expression
// `callee` = The variable which should evaluate to a function
// `args` = The argument array
// `count` = Count of arguments
// Return => New Call Expression as Expression pointer or NULL
PExpr *NewCallExpr(Token *op, PExpr *callee, PExpr **args, int count);

// Create New (Empty) Statement
// Return => New Expression with type `type` or NULL in case of failure
PStmt *NewStmt(PStmtType type);

// Free Statement
void FreeStmt(PStmt * stmt);

// New (Debug) Print Statement
// `op` = The `print` token
// `value` = The expression to print
// Return => New Print Statement as Statement pointer or NULL
PStmt *NewPrintStmt(Token *op, PExpr *value);

// New (Naked) Expression Statement
// `op` = The last token the expression.
// The first expression is not used as single token expression without any
// statements can cause the `op` being NULL
// `value` = The actual expression
// Return => New Expression Statement as Statement pointer or NULL
PStmt *NewExprStmt(Token *op, PExpr *value);

// New Let (Variable Declaration) Statement
// `name` = Variable name
// `value` = The value to set
// Return => New Let Statement as Statement pointer or NULL
PStmt *NewLetStmt(Token *name, PExpr *value);

// New Block Statement
// In case of braced closure {....} `op` is the left brace
// When used for functions or while loops `op` is the last `END` token
// When used for If Statement, the `op` can be either `ELSE` or `END` depending
// on the branch.
// Return => New Block Statement as Statement pointer or NULL
PStmt *NewBlockStmt(Token *op, PStmt **stmts);

// New If Statement
// `op` = The `IF` token
// `cond` = Conditional Expression
// `then` = Then branch
// `elseB` = Else branch, it is optional and can be NULL
// Return => New If Statement as Statement pointer or NULL
PStmt *NewIfStmt(Token *op, PExpr *cond, PStmt *then, PStmt *elseB);

// New While Statement
// `op` = The `WHILE` token
// `cond` = The eonditional expression
// `body` = The loop body. Always will be a Block Statement.
// Return => New While Statement as Statement pointer or NULL
PStmt *NewWhileStmt(Token *op, PExpr *cond, PStmt *body);

// New Return Statement
// `op` = The `RETURN` token
// `value` = The actual value. It is optional and can be NULL
// Return => New Return Statement as Statement pointer or NULL
PStmt *NewReturnStmt(Token *op, PExpr *value);

// New Break Statement
// `op` = The `BREAK` token
// Only usable in While loops
// Return => New Break Statement as Statement pointer or NULL
PStmt *NewBreakStmt(Token *op);

// New Function Statement
// `name` = The name token of the function
// `params` = Token array of parameters
// `body` = Function body. Always will be a Block Statement
// `count` = Number of parameters
// Return => New Function Statement as Statement pointer or NULL
PStmt *NewFuncStmt(Token *name, Token **params, PStmt *body, int count);

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
