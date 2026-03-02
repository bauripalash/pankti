#ifndef PANKTI_COMPILER_ERRORS_H
#define PANKTI_COMPILER_ERRORS_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#define CMP_ERR_IME "Internal Memory Error : "
#define CMP_ERR_JUMP_TOO_BIG "Condition jump is too big"
#define CMP_ERR_VAR_OWN_INIT "Cannot read variable in its own initialization"
#define CMP_ERR_FAIL_TOP_LVL_STMT "Failed to compile top level statement in script"
#define CMP_ERR_IME_FAIL_STROBJ_LIT CMP_ERR_IME "Failed to create string object while compiling literal" 
#define CMP_ERR_INVALID_EXPR "Invalid literal expression found"
#define CMP_ERR_LEFT_BIN_EXPR "Failed to compile left side of the binary expression"
#define CMP_ERR_RIGHT_BIN_EXPR "Failed to compile right side of the binary expression"
#define CMP_ERR_LEFT_LGC_EXPR "Failed to compile left side of the logical expression"
#define CMP_ERR_RIGHT_LGC_EXPR "Failed to compile right side of the logical expression"
#define CMP_ERR_UNARY_EXPR "Failed to compile unary expression"
#define CMP_ERR_ARR_ITEM_EXPR "Failed to compile array item expression"
#define CMP_ERR_MAP_KEY_EXPR "Failed to compile map key expression"
#define CMP_ERR_MAP_VAL_EXPR "Failed to compile map value expression"
#define CMP_ERR_IME_FAIL_STR_IDENT CMP_ERR_IME "Failed to create string object while compiling identifier"
#define CMP_ERR_ASN_VAL "Failed to compile value expression of assignment"
#define CMP_ERR_SUBS_ASN_VAL "Failed to compile value expression of subscript assignment"
#define CMP_ERR_SUBS_ASN_IDX "Failed to compile index expression of subscript assignment"
#define CMP_ERR_SUBS_ASN_ASNVAL "Failed to compile assignment value expression of subscript assignment"
#define CMP_ERR_CALL_CALLEE "Failed to compile callee expression of function call"
#define CMP_ERR_CALL_ARG "Failed to compile argument expression of function call"
#define CMP_ERR_SUBS_VAL "Failed to compile value expression of subscript"
#define CMP_ERR_SUBS_IDX "Failed to compile index expression of subscript"
#define CMP_ERR_MOD_EXPR "Failed to compile module expression"
#define CMP_ERR_EXPR_STMT "Failed to compile expression statement"
#define CMP_ERR_DBG_STMT "Failed to compile debug statement"
#define CMP_ERR_VAR_EXIST_SCOPE "Same variable `%s` exists in the same scope"
#define CMP_ERR_TOO_MANY_LOCAL "Too many local variable declared in local scope"
#define CMP_ERR_LET_STMT "Failed to compile let statement"
#define CMP_ERR_BLK_STMT "Failed to compile block statement"
#define CMP_ERR_IF_COND "Failed to compile if condition expression"
#define CMP_ERR_IF_THEN "Failed to compile then statements"
#define CMP_ERR_IF_ELSE "Failed to compile else statements"
#define CMP_ERR_IME_FAIL_LPCTX_WHL "Failed to create loop context of for while statement"
#define CMP_ERR_WHL_COND "Failed to compile while condition expression"
#define CMP_ERR_WHL_BODY "Failed to compile while statement body"
#define CMP_ERR_IME_FAIL_ACS_LPCTX_BRK "Failed to access loop context while compiling break statement"
#define CMP_ERR_IME_FAIL_ACS_LPCTX_CNT "Failed to access loop context while compiling continue statement"
#define CMP_ERR_FNC_BODY_STMT "Failed to compile function body statement"
#define CMP_ERR_IME_FAIL_FNC_CMP "Failed to create function compiler"
#define CMP_ERR_FNC_BODY "Failed to compile function body"
#define CMP_ERR_FNC_ACS_CMPFNC "Failed to access compiled function"
#define CMP_ERR_RET_TOP_LVL "Cannot use return in top level script"
#define CMP_ERR_RET_VAL "Failed to compile return value"
#define CMP_ERR_IMPRT_PATH "Failed to compile import path expression"
/* clang-format on */
#ifdef __cplusplus
}
#endif

#endif
