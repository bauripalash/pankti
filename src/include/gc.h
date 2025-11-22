#ifndef GC_H
#define GC_H

#include "ast.h"
#include "object.h"
#include "token.h"
#include <stddef.h>
#include <stdint.h>
#include <time.h>
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
// #define DEBUG_GC

// Pankti Garbage Collector
typedef struct Pgc {
    bool disable;
    bool stress;
    PObj *objects;
    PStmt *stmts;
    size_t nextGc;
	uint64_t timestamp;
} Pgc;

Pgc *NewGc();
void FreeGc(Pgc *gc);

// Create a New Empty Object.
// `type` = Type of statement
PObj *NewObject(Pgc *gc, PObjType type);

// Free the Object
// Handle all underlying values according to function type.
void FreeObject(Pgc *gc, PObj *o);

// Create New String Object
// `name` = Token (optional if virtual, created in runtime)
// `value` = String value
// `virt` = Is the string virtual aka. created on runtime
PObj *NewStrObject(Pgc *gc, Token *name, char *value, bool virt);

// Create Function Statement Object
// `name` = Name of the function
// `params` = Token array of parameters
// `body` = Body of function. Statement will always be Block Statement
// `env` = Environment
// `count` = Count of parameters
PObj *NewFuncObject(
    Pgc *gc, Token *name, Token **params, PStmt *body, void *env, size_t count
);

// Create New Array Object
PObj *NewArrayObject(Pgc *gc, Token *op, PValue *items, size_t count);

// Create New Map Object
PObj *NewMapObject(Pgc *gc, Token *op);

// Create New Native Function Object
PObj *NewNativeFnObject(Pgc *gc, Token *name, NativeFn fn, int arity);

// Create New (Empty) Expression
// `type` = Expression Type
// Return => New Expression with type `type` or NULL in case of failure
PExpr *NewExpr(Pgc *gc, PExprType type, Token *op);

// Free Expression
void FreeExpr(Pgc *gc, PExpr *expr);

// New Binary Expression. Type : `EXPR_BINARY`
// `op` = Raw operator token
// `left` = Left hand side expression
// `right` = Right hand side expression
// Return => New Binary Expression as Expression pointer or NULL
PExpr *NewBinaryExpr(Pgc *gc, PExpr *left, Token *op, PExpr *right);

// New Unary Expression. Type : `EXPR_UNARY`
// `op` = Raw operator token
// `right` = Right hand side [Original] expression
// Return => New Unary Expression as Expression pointer or NULL
PExpr *NewUnary(Pgc *gc, Token *op, PExpr *right);

// New Literal Expression (ie. Bool, String, Nil etc). Type : `EXPR_LITERAL`
// `op` = Raw token
// `type` = Literal type
// Return => New Literal Expression as Expression pointer or NULL
PExpr *NewLiteral(Pgc *gc, Token *op, ExpLitType type);

// New Grouping Expression. Type : `EXPR_GROUPING`
// `(<expr>)`
// Return => New Grouping Expression as Expression pointer or NULL
PExpr *NewGrouping(Pgc *gc, Token *op, PExpr *expr);

// New Variable Expression. Type : `EXPR_VARIABLE`
// `name` = Variable name
// Return => New Variable Expression as Expression pointer or NULL
PExpr *NewVarExpr(Pgc *gc, Token *name);

// New Assignment Expression. Type : `EXPR_ASSIGN`
// Set `value` to a preexisting variable with name being `name`
// Return => New Assignment Expression as Expression pointer or NULL
PExpr *NewAssignment(Pgc *gc, Token *op, PExpr *name, PExpr *value);

// New Logical Expression (ie. And, Or). Type : `EXPR_LOGICAL`
// Return => New Logical Expression as Expression pointer or NULL
PExpr *NewLogical(Pgc *gc, PExpr *left, Token *op, PExpr *right);

// New (Function) Calling Expression. Type : `EXPR_CALL`
// `op` = Right parentheses of the call expression
// `callee` = The variable which should evaluate to a function
// `args` = The argument array
// `count` = Count of arguments
// Return => New Call Expression as Expression pointer or NULL
PExpr *NewCallExpr(Pgc *gc, Token *op, PExpr *callee, PExpr **args, size_t count);

// New Array Expression
PExpr *NewArrayExpr(Pgc *gc, Token *op, PExpr **items, size_t count);

// Create New HashMap Expression
PExpr *NewMapExpr(Pgc *gc, Token *op, PExpr **items, size_t count);

PExpr *NewSubscriptExpr(Pgc *gc, Token *op, PExpr *value, PExpr *index);
PExpr *NewModgetExpr(Pgc *gc, Token *op, PExpr *module, Token *child);

// Create New (Empty) Statement
// Return => New Expression with type `type` or NULL in case of failure
PStmt *NewStmt(Pgc *gc, PStmtType type, Token *op);

// Free Statement
void FreeStmt(Pgc *gc, PStmt *stmt);

// New (Debug) Print Statement
// `op` = The `print` token
// `value` = The expression to print
// Return => New Print Statement as Statement pointer or NULL
PStmt *NewPrintStmt(Pgc *gc, Token *op, PExpr *value);

// New (Naked) Expression Statement
// `op` = The last token the expression.
// The first expression is not used as single token expression without any
// statements can cause the `op` being NULL
// `value` = The actual expression
// Return => New Expression Statement as Statement pointer or NULL
PStmt *NewExprStmt(Pgc *gc, Token *op, PExpr *value);

// New Let (Variable Declaration) Statement
// `name` = Variable name
// `value` = The value to set
// Return => New Let Statement as Statement pointer or NULL
PStmt *NewLetStmt(Pgc *gc, Token *name, PExpr *value);

// New Block Statement
// In case of braced closure {....} `op` is the left brace
// When used for functions or while loops `op` is the last `END` token
// When used for If Statement, the `op` can be either `ELSE` or `END` depending
// on the branch.
// Return => New Block Statement as Statement pointer or NULL
PStmt *NewBlockStmt(Pgc *gc, Token *op, PStmt **stmts);

// New If Statement
// `op` = The `IF` token
// `cond` = Conditional Expression
// `then` = Then branch
// `elseB` = Else branch, it is optional and can be NULL
// Return => New If Statement as Statement pointer or NULL
PStmt *NewIfStmt(Pgc *gc, Token *op, PExpr *cond, PStmt *then, PStmt *elseB);

// New While Statement
// `op` = The `WHILE` token
// `cond` = The eonditional expression
// `body` = The loop body. Always will be a Block Statement.
// Return => New While Statement as Statement pointer or NULL
PStmt *NewWhileStmt(Pgc *gc, Token *op, PExpr *cond, PStmt *body);

// New Return Statement
// `op` = The `RETURN` token
// `value` = The actual value. It is optional and can be NULL
// Return => New Return Statement as Statement pointer or NULL
PStmt *NewReturnStmt(Pgc *gc, Token *op, PExpr *value);

// New Break Statement
// `op` = The `BREAK` token
// Only usable in While loops
// Return => New Break Statement as Statement pointer or NULL
PStmt *NewBreakStmt(Pgc *gc, Token *op);

// New Function Statement
// `name` = The name token of the function
// `params` = Token array of parameters
// `body` = Function body. Always will be a Block Statement
// `count` = Number of parameters
// Return => New Function Statement as Statement pointer or NULL
PStmt *NewFuncStmt(
    Pgc *gc, Token *name, Token **params, PStmt *body, size_t count
);

// New Import Statement
// `op` = The `import` token
// `iname` = The custom name for the import
// `ipath` = The import path; must evaluate to a string
// Return = New Import Statement as Statement pointer or NULL
PStmt *NewImportStmt(Pgc *gc, Token *op, Token *iname, PExpr *ipath);

#ifdef __cplusplus
}
#endif

#endif
