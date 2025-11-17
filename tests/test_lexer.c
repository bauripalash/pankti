#include <stdbool.h>
#include "../src/include/lexer.h"
#include "../src/include/utils.h"
#include "include/utest.h"
#include "../src/external/stb/stb_ds.h"

#define SetupLexer(src)\
	utest_fixture->lx->source = src;\
	ResetLexer(utest_fixture->lx);\
	ScanTokens(utest_fixture->lx);\


#define CheckTokens(lx, ttypes)\
	int count = ArrCount(types);\
	Token ** tokens = lx->tokens;\
	int ogCount = arrlen(tokens);\
	ASSERT_EQ_MSG(count, ogCount, "Token Count Doesnot match!");\
	for (int i = 0; i < count; i++) {\
		TokenType expected = ttypes[i];\
		TokenType got = tokens[i]->type;\
		ASSERT_EQ(expected, got);\
	}\


struct LexerTest{
	Lexer * lx;
};

UTEST_F_SETUP(LexerTest){
	utest_fixture->lx = NewLexer("");
	MakeLexerRaw(utest_fixture->lx, true);
}

UTEST_F_TEARDOWN(LexerTest){
	FreeLexer(utest_fixture->lx);
}

UTEST_F(LexerTest, Operator){
	SetupLexer("1+2.0-3.5*4.999/5.1**6.01")

	TokenType types[] = {
		T_NUM,
		T_PLUS,
		T_NUM,
		T_MINUS,
		T_NUM,
		T_ASTR,
		T_NUM,
		T_SLASH,
		T_NUM,
		T_EXPONENT,
		T_NUM,
		T_EOF
	};

	CheckTokens(utest_fixture->lx, types);
}
