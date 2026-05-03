#ifndef PANKTI_COMPILER_ERRORS_H
#define PANKTI_COMPILER_ERRORS_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#define CMP_ERR_IME "Internal Memory Error : "
#define CMP_ERR_JUMP_TOO_BIG "Condition jump is too big" //done
#define CMP_ERR_VAR_OWN_INIT "Cannot read variable in its own initialization" //done
#define CMP_ERR_FAIL_TOP_LVL_STMT "Failed to compile top level statement in script" //done
#define CMP_ERR_IME_FAIL_STROBJ_LIT CMP_ERR_IME "Failed to create string object while compiling literal" //done
#define CMP_ERR_INVALID_EXPR "Invalid literal expression found" //done
#define CMP_ERR_LEFT_BIN_EXPR "Failed to compile left side of the binary expression" //done
#define CMP_ERR_RIGHT_BIN_EXPR "Failed to compile right side of the binary expression" //done
#define CMP_ERR_LEFT_LGC_EXPR "Failed to compile left side of the logical expression" //done
#define CMP_ERR_RIGHT_LGC_EXPR "Failed to compile right side of the logical expression" //done
#define CMP_ERR_UNARY_EXPR "Failed to compile unary expression" //done
#define CMP_ERR_ARR_ITEM_EXPR "Failed to compile array item expression" //done
#define CMP_ERR_MAP_KEY_EXPR "Failed to compile map key expression" //done
#define CMP_ERR_MAP_VAL_EXPR "Failed to compile map value expression" //done
#define CMP_ERR_IME_FAIL_STR_IDENT CMP_ERR_IME "Failed to create string object while compiling identifier" //donex
#define CMP_ERR_ASN_VAL "Failed to compile value expression of assignment" //done
#define CMP_ERR_SUBS_ASN_VAL "Failed to compile value expression of subscript assignment" //done
#define CMP_ERR_SUBS_ASN_IDX "Failed to compile index expression of subscript assignment" //done
#define CMP_ERR_SUBS_ASN_ASNVAL "Failed to compile assignment value expression of subscript assignment" //done
#define CMP_ERR_CALL_CALLEE "Failed to compile callee expression of function call" //done
#define CMP_ERR_CALL_ARG "Failed to compile argument expression of function call" //done
#define CMP_ERR_SUBS_VAL "Failed to compile value expression of subscript" //done
#define CMP_ERR_SUBS_IDX "Failed to compile index expression of subscript" //done
#define CMP_ERR_MOD_EXPR "Failed to compile module expression" //done
#define CMP_ERR_EXPR_STMT "Failed to compile expression statement" //done
#define CMP_ERR_DBG_STMT "Failed to compile debug statement" //done
#define CMP_ERR_VAR_EXIST_SCOPE "Same variable `%s` exists in the same scope" //done
#define CMP_ERR_TOO_MANY_LOCAL "Too many local variable declared in local scope" //done
#define CMP_ERR_LET_STMT "Failed to compile let statement" //done
#define CMP_ERR_BLK_STMT "Failed to compile block statement" //done
#define CMP_ERR_IF_COND "Failed to compile if condition expression" //done
#define CMP_ERR_IF_THEN "Failed to compile then statements" //done
#define CMP_ERR_IF_ELSE "Failed to compile else statements" //done
#define CMP_ERR_IME_FAIL_LPCTX_WHL "Failed to create loop context of for while statement" //done
#define CMP_ERR_WHL_COND "Failed to compile while condition expression" //done
#define CMP_ERR_WHL_BODY "Failed to compile while statement body" //done
#define CMP_ERR_IME_FAIL_ACS_LPCTX_BRK "Failed to access loop context while compiling break statement" //done
#define CMP_ERR_IME_FAIL_ACS_LPCTX_CNT "Failed to access loop context while compiling continue statement" //done
#define CMP_ERR_FNC_BODY_STMT "Failed to compile function body statement" //done
#define CMP_ERR_IME_FAIL_FNC_CMP "Failed to create function compiler" //done
#define CMP_ERR_FNC_BODY "Failed to compile function body" //done
#define CMP_ERR_FNC_ACS_CMPFNC "Failed to access compiled function" //done
#define CMP_ERR_RET_TOP_LVL "Cannot use return in top level script" //done
#define CMP_ERR_RET_VAL "Failed to compile return value" //done
#define CMP_ERR_IMPRT_PATH "Failed to compile import path expression" //done

#define CMP_ERR_SESC_UNKN_ESC "Unknown escape sequence found in string"
#define CMP_ERR_SESC_INVLD_HEX "Invalid hex character found in string escape sequence"
#define CMP_ERR_SESC_BFR_NOT_ENGH "Internal Error : Failed to process string escape"
#define CMP_ERR_SESC_FNSH_ERLY "Incomplete escape sequence found at the end of the string"
#define CMP_ERR_SESC_NO_LO_SRGT "While reading \\uHHHH sequence another low surrogate \\uHHHH sequence as expected but not found"
#define CMP_ERR_SESC_LN_LOW_SURROGATE "While reading \\uHHHH sequence only a lone low surrogate was found"
#define CMP_ERR_SESC_INVLD_LO_SRGT "While reading \\uHHHH a invalid low surrogate surrogate was found after a high surrogate"
#define CMP_ERR_SESC_INVALID_8_CP "Found invalid codepoint in \\uHHHHHHHH sequence"
#define CMP_ERR_SESC_NUL_PTR "Internal Error: Invalid Memory allocation for output in reading escape sequences"
/* clang-format on */
#ifdef __cplusplus
}
#endif

#endif
