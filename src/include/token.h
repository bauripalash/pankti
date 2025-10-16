#ifndef TOKEN_H
#define TOKEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

// Token Types
typedef enum TokenType{
	// Token `(`
	T_LEFT_PAREN,
	
	// Token `)`
	T_RIGHT_PAREN,

	// Token `{`
	T_LEFT_BRACE,

	// Token `}`
	T_RIGHT_BRACE,

	// Token `[`
	T_LS_BRACKET,

	// Token `]`
	T_RS_BRACKET,

	// Token `,`
	T_COMMA,
	// Token `.`
	T_DOT,
	// Token `+`
	T_PLUS,
	// Token `-`
	T_MINUS,
	// Token `:`
	T_COLON,
	// Token `<Undecided>`
	T_MOD,
	// Token `;`
	T_SEMICOLON,
	// Token `/`
	T_SLASH,
	// Token `*`
	T_ASTR,

	// Token `!`
	T_BANG,
	// Token `!=`
	T_BANG_EQ,
	// Token `=`
	T_EQ,
	// Token `==`
	T_EQEQ,
	// Token `>`
	T_GT,
	// Token `>=`
	T_GTE,
	// Token `<`
	T_LT,
	// Token `<=`
	T_LTE,

	// Token : Identifier `myvar = 100`
	T_IDENT,
	// Token : String `"<str>"`
	T_STR,
	// Token : Number `100.00`
	T_NUM,

	// Token : Keyword `let/dhori`
	T_LET,
	// Token : Keyword `and/ebong`
	T_AND,
	// Token : Keyword `or/ba`
	T_OR,
	// Token : Keyword `func/kaj`
	T_FUNC,
	// Token : Keyword `if/jodi`
	T_IF,
	// Token : Keyword `then/tahole`
	T_THEN,
	// Token : Keyword `else/nahole`
	T_ELSE,
	// Token : Keyword `end/sesh`
	T_END,
	// Token : Keyword `while/jotokhon`
	T_WHILE,
	// Token : Keyword `do/koro`
	T_DO,
	// Token : Keyword `break/bhango`
	T_BREAK,
	// Token : Keyword `nil`
	T_NIL,
	// Token : Keyword `true/sotti`
	T_TRUE,
	// Token : Keyword `false/mittha`
	T_FALSE,
	// Token : Keyword `return/ferot`
	T_RETURN,
	// Token : Keyword `import/anoyon`
	T_IMPORT,
	// Token : Keyword `panic/golmal`
	T_PANIC,
	// Token : Keyword `len/ayoton`
	T_LEN,
	// Token : Keyword `print/print`
	T_PRINT,

	// Token : End of File
	T_EOF
}TokenType;

// Get Token Type as String
char * TokTypeToStr(TokenType type);
// Print Token Type
void PrintTokType(TokenType type);
// Is Token Double character?
bool IsDoubleTok(TokenType type);

// Token Object
typedef struct Token{
	// Token Type
	TokenType type;
	// Optional Lexeme : Referenced from source directly using string indexing.
	// Only used for Keywords, identifiers, strings, numbers.
	// Can be NULL
	char * lexeme;
	// Line number
	long line;
	// Column number. Distance from first character of the line
	long col;
	// length of the token lexeme, if lexeme is optional depends of how many 
	// characters the token contains need. For example, T_EQ is 1, T_EQEQ is 2
	long len;
	// Hash for the token lexeme, expect for Keywords, identifiers, strings,
	// numbers it will be 0 (zero)
	uint32_t hash;
}Token;

// Create an empty token.
// `type` = Token type
Token * NewToken(TokenType type);
// Free the token object
void FreeToken(Token * token);
// Set token lexeme. Calculate and set hash.
bool SetTokenLexeme(Token * token, char * str);
// Print Token
void PrintToken(const Token * token);
// Print operator type tokens. Special formatting is done as lexeme is NULL.
void PrintOpToken(const Token * token);

#ifdef __cplusplus
}
#endif

#endif
