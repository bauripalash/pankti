#ifndef TOKEN_H
#define TOKEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ptypes.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Token Types
typedef enum PTokenType {
    // Token `(`
    T_LEFT_PAREN = 1,

    // Token `)`
    T_RIGHT_PAREN = 2,

    // Token `{`
    T_LEFT_BRACE = 3,

    // Token `}`
    T_RIGHT_BRACE = 4,

    // Token `[`
    T_LS_BRACKET = 5,

    // Token `]`
    T_RS_BRACKET = 6,

    // Token `,`
    T_COMMA = 7,
    // Token `.`
    T_DOT = 8,
    // Token `+`
    T_PLUS = 9,
    // Token `-`
    T_MINUS = 10,
    // Token `:`
    T_COLON = 11,
    // Token `<Undecided>`
    T_MOD = 12,
    // Token `;`
    T_SEMICOLON = 13,
    // Token `/`
    T_SLASH = 14,
    // Token `*`
    T_ASTR = 15,
    // Token `**`
    T_EXPONENT = 16,

    // Token `!`
    T_BANG = 17,
    // Token `!=`
    T_BANG_EQ = 18,
    // Token `=`
    T_EQ = 19,
    // Token `==`
    T_EQEQ = 20,
    // Token `>`
    T_GT = 21,
    // Token `>=`
    T_GTE = 22,
    // Token `<`
    T_LT = 23,
    // Token `<=`
    T_LTE = 24,

    // Token : Identifier `myvar = 100`
    T_IDENT = 25,
    // Token : String `"<str>"`
    T_STR = 26,
    // Token : Number `100.00`
    T_NUM = 27,

    // Token : Keyword `let/dhori`
    T_LET = 28,
    // Token : Keyword `and/ebong`
    T_AND = 29,
    // Token : Keyword `or/ba`
    T_OR = 30,
    // Token : Keyword `func/kaj`
    T_FUNC = 31,
    // Token : Keyword `if/jodi`
    T_IF = 32,
    // Token : Keyword `then/tahole`
    T_THEN = 33,
    // Token : Keyword `else/nahole`
    T_ELSE = 34,
    // Token : Keyword `end/sesh`
    T_END = 35,
    // Token : Keyword `while/jotokhon`
    T_WHILE = 36,
    // Token : Keyword `do/koro`
    T_DO = 37,
    // Token : Keyword `break/bhango`
    T_BREAK = 38,
    // Token : Keyword `nil`
    T_NIL = 39,
    // Token : Keyword `true/sotti`
    T_TRUE = 40,
    // Token : Keyword `false/mittha`
    T_FALSE = 41,
    // Token : Keyword `return/ferot`
    T_RETURN = 42,
    // Token : Keyword `import/anoyon`
    T_IMPORT = 43,
    // Token : Keyword `panic/golmal`
    T_PANIC = 44,
    T_DEBUG = 45,
    // Token : End of File
    T_EOF = 47
} PTokenType;

// Get Token Type as String
char *TokTypeToStr(PTokenType type);
// Print Token Type
void PrintTokType(PTokenType type);
// Is Token Double character?
bool IsDoubleTok(PTokenType type);

// Token Object
typedef struct Token {
    // Token Type
    PTokenType type;
    // Optional Lexeme : Referenced from source directly using string indexing.
    // Only used for Keywords, identifiers, strings, numbers.
    // Can be NULL
    char *lexeme;
    // Line number
    u64 line;
    // Column number. Distance from first character of the line
    u64 col;
    // length of the token lexeme, if lexeme is optional depends of how many
    // characters the token contains need. For example, T_EQ is 1, T_EQEQ is 2
    u64 len;
    // Hash for the token lexeme. expect for Keywords, identifiers, strings,
    // numbers it will be 0 (zero)
    u64 hash;
} Token;

// Create an empty token.
// `type` = Token type
Token *NewToken(PTokenType type);
// Free the token object
void FreeToken(Token *token);
// Set token lexeme. Calculate and set hash.
bool SetTokenLexeme(Token *token, char *str);
// Print Token
void PrintToken(const Token *token);
// Print operator type tokens. Special formatting is done as lexeme is NULL.
void PrintOpToken(const Token *token);

#ifdef __cplusplus
}
#endif

#endif
