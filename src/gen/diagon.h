#ifndef PANKTI_DIAGON_H
#define PANKTI_DIAGON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef enum PanDiagCat{
	PAN_DIAG_LEXER = 0,
	PAN_DIAG_PARSER = 1,
	PAN_DIAG_COMPILER = 2,
	PAN_DIAG_RUNTIME = 3,
	PAN_DIAG_STRESCAPE = 4,
}PanDiagCat;

typedef enum PanDiagSeverity{
	PAN_DIAG_SEV_WARN = 0,
	PAN_DIAG_SEV_ERROR = 1,
}PanDiagSeverity;

typedef enum PanDiagCode{
    // অপরিচিত অক্ষর পাওয়া গেছে `%s`
    LEXER_INVALID_CHAR,
    // অপরিচিত অক্ষর পাওয়া গেছে এই অবস্থানেঃ %llu নং লাইনে %llu নং স্তম্ভে
    LEXER_INVALID_CHAR_AT,
    // অভ্যন্তরীণ গোলমাল: যৌক্তিক রাশিমালা তৈরি বিফল হয়েছে
    PARSER_IME_LOGICAL_EXPR,
    // অভ্যন্তরীণ গোলমাল: মান নির্ধারণ রাশিমালা তৈরি বিফল হয়েছে
    PARSER_IME_ASSIGN_EXPR,
    // অভ্যন্তরীণ গোলমাল: দ্বিমুখী রাশিমালা তৈরি বিফল হয়েছে
    PARSER_IME_BINARY_EXPR,
    // অভ্যন্তরীণ গোলমাল: একমুখী রাশিমালা তৈরি বিফল হয়েছে
    PARSER_IME_UNARY_EXPR,
    // অভ্যন্তরীণ গোলমাল: সূচকীয় রাশিমালা তৈরি বিফল হয়েছে
    PARSER_IME_SUBSCRIPT_EXPR,
    // অভ্যন্তরীণ গোলমাল: সত্যমান রাশি তৈরি বিফল হয়েছে
    PARSER_IME_BOOL_EXPR,
    // অভ্যন্তরীণ গোলমাল: নিল রাশি তৈরি বিফল হয়েছে
    PARSER_IME_NIL_EXPR,
    // অভ্যন্তরীণ গোলমাল: সংখ্যা রাশি তৈরি বিফল হয়েছে
    PARSER_IME_NUM_EXPR,
    // অভ্যন্তরীণ গোলমাল: লেখা রাশি তৈরি বিফল হয়েছে
    PARSER_IME_STR_EXPR,
    // অভ্যন্তরীণ গোলমাল: চলরাশি তৈরি বিফল হয়েছে
    PARSER_IME_IDENT_EXPR,
    // অভ্যন্তরীণ গোলমাল: গুচ্ছ রাশিমালা তৈরি বিফল হয়েছে
    PARSER_IME_GROUP_EXPR,
    // অভ্যন্তরীণ গোলমাল: তালিকা রাশিমালা তৈরি বিফল হয়েছে
    PARSER_IME_ARRAY_EXPR,
    // অভ্যন্তরীণ গোলমাল: ছক রাইমালা তৈরি বিফল হয়েছে
    PARSER_IME_MAP_EXPR,
    // অভ্যন্তরীণ গোলমাল: কাজের ডাক রাশিমালা তৈরি বিফল হয়েছে
    PARSER_IME_CALL_EXPR,
    // অভ্যন্তরীণ গোলমাল: ফেরাও বিবৃতি তৈরি বিফল হয়েছে
    PARSER_IME_RETURN_STMT,
    // অভ্যন্তরীণ গোলমাল: ভাঙো বিবৃতি তৈরি বিফল হয়েছে
    PARSER_IME_BREAK_STMT,
    // অভ্যন্তরীণ গোলমাল: চালাও বিবৃতি তৈরি বিফল হয়েছে
    PARSER_IME_CONTINUE_STMT,
    // অভ্যন্তরীণ গোলমাল: কাজ বিবৃতি তৈরি বিফল হয়েছে
    PARSER_IME_FUNC_STMT,
    // অভ্যন্তরীণ গোলমাল: আনয়ন বিবৃতি তৈরি বিফল হয়েছে
    PARSER_IME_IMPORT_STMT,
    // অভ্যন্তরীণ গোলমাল: যদি-তাহলে বিবৃতি তৈরি বিফল হয়েছে
    PARSER_IME_IF_STMT,
    // অভ্যন্তরীণ গোলমাল: যতক্ষণ-করো বিবৃতি তৈরি বিফল হয়েছে
    PARSER_IME_WHILE_STMT,
    // অভ্যন্তরীণ গোলমাল: চলরাশি তৈরি বিবৃতি তৈরি বিফল হয়েছে
    PARSER_IME_LET_STMT,
    // অভ্যন্তরীণ গোলমাল: রাশিমালা বিবৃতি তৈরি বিফল হয়েছে
    PARSER_IME_EXPR_STMT,
    // অভ্যন্তরীণ গোলমাল: পরিচিতি বিবৃতি তৈরি বিফল হয়েছে
    PARSER_IME_DEBUG_STMT,
    // অভ্যন্তরীণ গোলমাল: বিবৃতি গুচ্ছ তৈরি বিফল হয়েছে
    PARSER_IME_BLOCK_STMT,
    // অভ্যন্তরীণ গোলমাল: উৎসের সদস্য রাশি তৈরি বিফল হয়েছে
    PARSER_IME_MOD_CHILD,
    // রাশিমালা পাওয়া উচিত ছিল
    PARSER_EXPECT_EXPR,
    // সেমিকোলন ';' পাওয়া উচিত ছিল
    PARSER_EXPECT_SEMICOLOR,
    // 'কাজ'/'func'/'kaj' এর পরে কাজের নাম পাওয়া উচিত ছিল
    PARSER_EXPECT_FUNC_NAME,
    // কাজের নামের পর একটি প্রথম বন্ধনী '(' পাওয়া উচিত ছিল
    PARSER_EXPECT_LPAREN_FUNC_NAME,
    // কাজের নামের পর কাজের প্রাপ্তিমান পাওয়া উচিত ছিল
    PARSER_EXPECT_FUNC_PARAM,
    // কাজের প্রাপ্তিমানগুলির পর (যদি থাকে) একটি প্রথম বন্ধনী ')' পাওয়া উচিত ছিল
    PARSER_EXPECT_FUNC_RPAREN,
    // কাজের ডাক রাশিমালাতে প্রেরণমানগুলির পর একটি প্রথম বন্ধনী ')' পাওয়া উচিত ছিল
    PARSER_EXPECT_CALL_RPAREN,
    // 'ধরি'/'let'/'dhor' -এর পর চলরাশির নাম পাওয়া উচিত ছিল
    PARSER_EXPECT_LET_IDENT,
    // চলরাশি তৈরির সময় চলরাশির নামের পর একটি সমান চিহ্ন '=' পাওয়া উচিত ছিল
    PARSER_EXPECT_EQ_LET_IDENT,
    // যদি-তাহলে বিবৃতির শর্তের পর 'তাহলে'/'then'/'tahole' পাওয়া উচিত ছিল
    PARSER_EXPECT_THEN_IFCOND,
    // বিবৃতি গুচ্ছের পর 'শেষ'/'end'/'shesh' পাওয়া উচিত
    PARSER_EXPECT_END_BLOCK,
    // বিবৃতি গুচ্ছের পর 'নাহলে'/'else'/'nahole' পাওয়া উচিত
    PARSER_EXPECT_NOOP_ELSE_BLOCK,
    // বিবৃতি গুচ্ছের পর 'শেষ'/'end'/'shesh' পাওয়া উচিত
    PARSER_EXPECT_NOOP_END_BLOCK,
    // বিবৃতি গুচ্ছের পর তৃতীয় বন্ধনী '}' পাওয়া উচিত ছিল
    PARSER_EXPECT_RBRACE_BLOCK,
    // যতক্ষণ-করো বিবৃতির শর্তের পর 'করো'/'do'/'koro' পাওয়া উচিত ছিল
    PARSER_EXPECT_DO_WHILECOND,
    // খালি ফেরাও এর পর একটি সেমিকোলন ';' পাওয়া উচিত ছিল
    PARSER_EXPECT_SEMICOLON_NILRETURN,
    // গুচ্ছ রাশিমালার পর প্রথম বন্ধনী ')' পাওয়া উচিত ছিল
    PARSER_EXPECT_RPAREN_GROUP,
    // ছকের সূচক-মান যুগলগুলির শেষে তৃতীয় বন্ধনী '}' পাওয়া উচিত ছিল
    PARSER_EXPECT_RBRACE_MAP,
    // ছকের সূচক-মান যুগলের পর কমা ',' পাওয়া উচিত ছিল
    PARSER_EXPECT_COMMA_MAPPAIR,
    // ছকের সূচকের পর কোলন ':' পাওয়া উচিত ছিল
    PARSER_EXPECT_COLON_MAPKEY,
    // তালিকার উপাদানগুলির শেষে দ্বিতীয় বন্ধনী ']' পাওয়া উচিত ছিল
    PARSER_EXPECT_RBRACKET_ARRAY,
    // তালিকার উপাদানের পর কমা ',' পাওয়া উচিত ছিল
    PARSER_EXPECT_COMMA_ARRAYITEM,
    // উৎসের সদস্যের নাম পাওয়া উচিত ছিল
    PARSER_EXPECT_MOD_CHILD,
    // সূচকীয় রাশির পর দ্বিতীয় বন্ধনী ']' পাওয়া উচিত ছিল
    PARSER_EXPECT_RBRACKET_SUBEXPR,
    // সূচকীয় মানে অবৈধ সূচক
    PARSER_INVALID_SUBEXPR_INDEX,
    // কাজ চালানোর রাশিমালাতে ২৫৫-টির বেশি প্রেরণ মান ব্যবহার করা যায় না
    PARSER_CALL_TOOMANY_ARGS,
    // অবৈধ মান নির্ধারণ রাশিমালা, শুধুমাত্র চলরাশি এবং সূচকীয় রাশির মান নির্ধারণ করা যায়
    PARSER_EXPECT_INVALID_ASSIGN,
    // ঘাত রাশিমালা পড়ার সময় দ্বিমুখী রাশিমালার ডান দিক থেকে অবৈধ রাশিমালা পাওয়া গেছে
    PARSER_INVALID_EXPO_RIGHT,
    // অবৈধ সংখ্যা পাওয়া গেছে
    PARSER_MALFORMED_NUMBER,

	PANDIAG_CODE_COUNT
}PanDiagCode;

typedef struct PanDiagInfo{
	PanDiagCode code;
	PanDiagCat category;
	PanDiagSeverity severity;
	bool formatted;
	const char * msg;
	const char * hint;
}PanDiagInfo;

const PanDiagInfo * DiagGetInfo(PanDiagCode code);
const char * DiagGetMsg(PanDiagCode code);
const char * DiagGetHint(PanDiagCode code);


#ifdef __cplusplus
}
#endif

#endif
