#ifndef PARSER_ERRORS_H
#define PARSER_ERRORS_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#define PARSER_ERR_IME "Internal Memory Error : "
#define PARSER_ERR_SEMICOLON "Expected Semicolon"
#define PARSER_ERR_IME_FAIL_LOGICAL_AT_AND PARSER_ERR_IME "Failed to create logical expression while reading 'and'"
#define PARSER_ERR_IME_FAIL_LOGICAL_AT_OR PARSER_ERR_IME "Failed to create logical expression while reading 'or'"
#define PARSER_ERR_IME_ASSIGN_EXPR PARSER_ERR_IME PARSER_ERR_IME "Failed to create assignment expression"
#define PARSER_ERR_INVALID_ASSIGN "Invalid assignment can only assign to variables and subscripts"
#define PARSER_ERR_IME_FAIL_BINARY_AT_EQ PARSER_ERR_IME "Failed to create binary expression when reading equality expression"
#define PARSER_ERR_IME_FAIL_BINARY_AT_COMPR PARSER_ERR_IME "Failed to create binary expression when reading comparison expression"
#define PARSER_ERR_IME_FAIL_BINARY_AT_ADDSUB PARSER_ERR_IME "Failed to create binary expression when reading addition/substraction expression"
#define PARSER_ERR_INVALID_FACT_EXPR "Invalid expression found when reading factor expression"
#define PARSER_ERR_IME_FAIL_BINARY_AT_MULDIV PARSER_ERR_IME "Failed to create binary expression when reading multiplication/division expression"
#define PARSER_ERR_IME_FAIL_UNARY PARSER_ERR_IME "Failed to create unary expression"
#define PARSER_ERR_INVALID_EXPR_EXPO "Invalid expression found in exponent expression"
#define PARSER_ERR_IME_FAIL_BINARY_AT_EXPO PARSER_ERR_IME "Failed to create binary expression when reading exponent expression"
#define PARSER_ERR_CALL_CANT_TOOMANY_ARGS "Can't have more than 255 arguments in function call"


#define PARSER_ERR_EXPECT_RPAREN_ARGS "Expected ')' after arguments"
#define PARSER_ERR_IME_FAIL_CALL_EXPR PARSER_ERR_IME "Failed to create call expression"
#define PARSER_ERR_INVALID_SUBS_IDX "Invalid subscript index"
#define PARSER_ERR_EXPECT_RSBRACK_SUBS "Expected ']' after subscript expression"
#define PARSER_ERR_IME_FAIL_SUBS_EXPR PARSER_ERR_IME "Failed to create subscript expression"

#define PARSER_ERR_EXPECT_MOD_CHILD "Expected child token for module"
#define PARSER_ERR_IME_FAIL_MOD_CHILD PARSER_ERR_IME "Failed to create module child get expression"
#define PARSER_ERR_EXPECT_COMMA_ARR_ITEM "Expected ',' after array item"
#define PARSER_ERR_EXPECT_RSBRACK_ARR "Expected ']' after array items"
#define PARSER_ERR_EXPECT_COLON_MAPKEY "Expected ':' after map key"
#define PARSER_ERR_EXPECT_COMMA_MAPPAIR "Expected ',' after map pair"
#define PARSER_ERR_EXPECT_RBRACE_MAP "Expected '}' after map"

#define PARSER_ERR_SESC_UNKN_ESC "Unknown escape sequence found in string"
#define PARSER_ERR_SESC_INVLD_HEX "Invalid hex character found in string escape sequence"
#define PARSER_ERR_SESC_BFR_NOT_ENGH "Internal Error : Failed to process string escape"
#define PARSER_ERR_SESC_FNSH_ERLY "Incomplete escape sequence found at the end of the string"
#define PARSER_ERR_SESC_NO_LO_SRGT "While reading \\uHHHH sequence another low surrogate \\uHHHH sequence as expected but not found"
#define PARSER_ERR_SESC_LN_LOW_SURROGATE "While reading \\uHHHH sequence only a lone low surrogate was found"
#define PARSER_ERR_SESC_INVLD_LO_SRGT "While reading \\uHHHH a invalid low surrogate surrogate was found after a high surrogate"
#define PARSER_ERR_SESC_INVALID_8_CP "Found invalid codepoint in \\uHHHHHHHH sequence"
#define PARSER_ERR_SESC_NUL_PTR "Internal Error: Invalid Memory allocation for output in reading escape sequences"
#define PARSER_ERR_IME_FAIL_BOOL_LIT PARSER_ERR_IME "Failed to create bool literal expression"
#define PARSER_ERR_IME_FAIL_NIL_LIT PARSER_ERR_IME "Failed to create nil literal expression"
#define PARSER_ERR_MALFRM_NUM "Invalid or malformed number found"
#define PARSER_ERR_IME_FAIL_NUM_LIT PARSER_ERR_IME "Failed to create number literal expression"
#define PARSER_ERR_IME_FAIL_STR_CHAR_LIT PARSER_ERR_IME "Failed to create string literal's characters"
#define PARSER_ERR_IME_FAIL_STR_LIT PARSER_ERR_IME "Failed to create string literal expression"
#define PARSER_ERR_IME_FAIL_IDENT_EXPR PARSER_ERR_IME "Failed to create identifier expression"
#define PARSER_ERR_EXPECT_RPAREN_GRP "Expected ')' after group expression"

#define PARSER_ERR_IME_FAIL_GRP PARSER_ERR_IME "Failed to create group expression"
#define PARSER_ERR_IME_FAIL_ARR PARSER_ERR_IME "Failed to create array expression"
#define PARSER_ERR_IME_FAIL_MAP PARSER_ERR_IME "Failed to create map expression"
#define PARSER_ERR_EXPECT_EXPR "Expected expression"
#define PARSER_ERR_IME_FAIL_EXPSTMT PARSER_ERR_IME "Failed to create expression statement"
#define PARSER_ERR_IME_FAIL_DBGSTMT PARSER_ERR_IME "Failed to create debug statement"
#define PARSER_ERR_EXPECT_IDENT_LET "Expected identifier after `let`"
#define PARSER_ERR_EXPECT_EQ_IDENT_LET "Expected '=' after identifier of let statement"
#define PARSER_ERR_IME_FAIL_LETSTMT PARSER_ERR_IME "Failed to create let statement"
#define PARSER_ERR_EXPECT_RBRACE_BLK PARSER_ERR_IME "Expected '}' after block statement"
#define PARSER_ERR_IME_FAIL_BLKSTMT PARSER_ERR_IME "Failed to create block statement"
#define PARSER_ERR_EXPECT_END_BLK "Expected 'end' after block"
#define PARSER_ERR_NOP_ELSE_BLOCK "Expected 'else' after block"
#define PARSER_ERR_NOP_END_BLOCK "Expected 'end' after block"
#define PARSER_ERR_EXPECT_THEN_IF "Expected 'then' after if expression"

#define PARSER_ERR_IME_FAIL_IFSTMT PARSER_ERR_IME "Failed to create if statement"

#define PARSER_ERR_EXPECT_DO_WHILE "Expected 'do' after while expression"
#define PARSER_ERR_IME_FAIL_WHLSTMT PARSER_ERR_IME "Failed to create while statement"
#define PARSER_ERR_EXPECT_SCOLON_EMT_RET "Expected ';' after empty return"
#define PARSER_ERR_IME_FAIL_RETSTMT PARSER_ERR_IME "Failed to create return statement"
#define PARSER_ERR_IME_FAIL_BRKSTMT PARSER_ERR_IME "Failed to create break statement"
#define PARSER_ERR_IME_FAIL_CNTSTMT PARSER_ERR_IME "Failed to create continue statement"
#define PARSER_ERR_EXPECT_FN_NAME_FUNC "Expected function name after 'func'"
#define PARSER_ERR_EXPECT_LPAREN_FN_NAME "Expected '(' after function name"
#define PARSER_ERR_EXPECT_PARAM_FNPRM "Expected parameters after function name"
#define PARSER_ERR_EXPECT_RPAREN_FN_PARM "Expected ')' after parameters list"

#define PARSER_ERR_IME_FAIL_FNCSTMT PARSER_ERR_IME "Failed to create function statement"
#define PARSER_ERR_IME_FAIL_IMPSTMT PARSER_ERR_IME "Failed to create import statement"
/* clang-format on */
#ifdef __cplusplus
}
#endif

#endif
