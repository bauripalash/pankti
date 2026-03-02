#ifndef PANKTI_COMPILER_ERRORS_H
#define PANKTI_COMPILER_ERRORS_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#define CMP_ERR_IME "Internal Memory Error : "
#define CMP_ERR_JUMP_TOO_BIG "Condition jump is too big"
#define CMP_ERR_VAR_OWN_INIT "Cannot read variable in its own statement"
#define CMP_ERR_VAR_EXIST_SCOPE "Same variable exists in this scope : %s"
#define CMP_ERR_IME_FAIL_LOOPCTX CMP_ERR_IME "Failed to create loop context"
#define CMP_ERR_BREAK_OUT_LOOP "Cannot use `break` outside of loops"
#define CMP_ERR_CONTINUE_OUT_LOOP "Cannot use `continue` outside of loops"
#define CMP_ERR_TOP_RETURN "Top level script cannot contain return"
/* clang-format on */
#ifdef __cplusplus
}
#endif

#endif
