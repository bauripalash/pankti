#include "../../src/external/stb/stb_ds.h"
#include "../../src/include/lexer.h"
#include "../../src/include/utils.h"
#include "include/utest.h"
#include <stdbool.h>

#define DEBUG_TEST_LEXER

#define SetupLexer(src)                                                        \
    utest_fixture->lx->source = src;                                           \
    ResetLexer(utest_fixture->lx);                                             \
    ScanTokens(utest_fixture->lx);

#define PrintAllTokens(toks, count)                                            \
    printf("==== Token ====\n");                                               \
    for (int i = 0; i < count; i++) {                                          \
        PrintToken(toks[i]);                                                   \
        printf("\n");                                                          \
    }                                                                          \
    printf("===============\n");

#define CheckTokens(lx, ttypes)                                                \
    int count = ArrCount(types);                                               \
    Token **tokens = lx->tokens;                                               \
    int ogCount = arrlen(tokens);                                              \
    ASSERT_EQ_MSG(count, ogCount, "Token Count Doesnot match!"); \
    for (int i = 0; i < count; i++) {                                          \
        TokenType expected = ttypes[i];                                        \
        TokenType got = tokens[i]->type;                                       \
        ASSERT_EQ_MSG(                                                         \
            expected, got,                                                     \
            StrFormat("%s != %s", TokTypeToStr(expected), TokTypeToStr(got))   \
        );                                                                     \
    }

struct LexerTest {
    Lexer *lx;
};

UTEST_F_SETUP(LexerTest) {
    utest_fixture->lx = NewLexer("");
    MakeLexerRaw(utest_fixture->lx, true);
}

UTEST_F_TEARDOWN(LexerTest) { FreeLexer(utest_fixture->lx); }

UTEST_F(LexerTest, Operator) {
    SetupLexer("1+2.0-3.5*4.999/5.1**6.01")

        TokenType types[] = {T_NUM, T_PLUS,  T_NUM, T_MINUS,    T_NUM, T_ASTR,
                             T_NUM, T_SLASH, T_NUM, T_EXPONENT, T_NUM, T_EOF};

    CheckTokens(utest_fixture->lx, types);
}

UTEST_F(LexerTest, EnglishKeywords) {
    char *src = "let and or end if then else while nil true false"
                " return func import panic do break";

    SetupLexer(src);
    TokenType types[] = {
        T_LET,  T_AND,    T_OR,    T_END,  T_IF,    T_THEN,
        T_ELSE, T_WHILE,  T_NIL,   T_TRUE, T_FALSE, T_RETURN,
        T_FUNC, T_IMPORT, T_PANIC, T_DO,   T_BREAK, T_EOF,
    };

    CheckTokens(utest_fixture->lx, types);
}

UTEST_F(LexerTest, PhoneticKeywords) {
    char *src = "dhori ebong ba sesh jodi tahole nahole jotokhon"
                " nil sotti mittha ferao kaj anoyon golmal koro bhango";

    SetupLexer(src);
    TokenType types[] = {
        T_LET,  T_AND,    T_OR,    T_END,  T_IF,    T_THEN,
        T_ELSE, T_WHILE,  T_NIL,   T_TRUE, T_FALSE, T_RETURN,
        T_FUNC, T_IMPORT, T_PANIC, T_DO,   T_BREAK, T_EOF,
    };

    CheckTokens(utest_fixture->lx, types);
}

UTEST_F(LexerTest, BengaliKeywords) {
    char *src = "ধরি এবং বা শেষ যদি তাহলে নাহলে যতক্ষণ নিল সত্যি মিথ্যা ফেরাও"
                " কাজ আনয়ন গোলমাল করো ভাঙো";
    SetupLexer(src);
    TokenType types[] = {
        T_LET,  T_AND,    T_OR,    T_END,  T_IF,    T_THEN,
        T_ELSE, T_WHILE,  T_NIL,   T_TRUE, T_FALSE, T_RETURN,
        T_FUNC, T_IMPORT, T_PANIC, T_DO,   T_BREAK, T_EOF,
    };

    CheckTokens(utest_fixture->lx, types);
}

UTEST_F(LexerTest, MixedKeywords) {
    char *src = "dhori and বা শেষ jodi তাহলে else jotokhon নিল সত্যি মিথ্যা"
                " ferao func আনয়ন panic koro ভাঙো";
    SetupLexer(src);
    TokenType types[] = {
        T_LET,  T_AND,    T_OR,    T_END,  T_IF,    T_THEN,
        T_ELSE, T_WHILE,  T_NIL,   T_TRUE, T_FALSE, T_RETURN,
        T_FUNC, T_IMPORT, T_PANIC, T_DO,   T_BREAK, T_EOF,
    };

    CheckTokens(utest_fixture->lx, types);
}
