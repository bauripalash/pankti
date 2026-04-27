#ifndef PARSER_ERRORS_H
#define PARSER_ERRORS_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#define PARSER_ERR_IME "অভ্যন্তরীণ গোলমাল : "
#define PARSER_ERR_SEMICOLON "সেমিকোলন ';' পাওয়া উচিত ছিল"
#define PARSER_ERR_IME_FAIL_LOGICAL_AT_AND PARSER_ERR_IME "'এবং'/'and'/'ebong' পড়ার সময় যৌক্তিক রাশিমালা তৈরি বিফল হয়েছে"
#define PARSER_ERR_IME_FAIL_LOGICAL_AT_OR PARSER_ERR_IME "'বা'/'or'/'ba' পড়ার সময় যৌক্তিক রাশিমালা তৈরি বিফল হয়েছে"
#define PARSER_ERR_IME_ASSIGN_EXPR PARSER_ERR_IME "মান নির্ধারণ রাশিমালা তৈরি বিফল হয়েছে"
#define PARSER_ERR_INVALID_ASSIGN "অবৈধ মান নির্ধারণ রাশিমালা, শুধুমাত্র চলরাশি এবং সূচকীয় রাশির মান নির্ধারণ করা যায়"
#define PARSER_ERR_IME_FAIL_BINARY_AT_EQ PARSER_ERR_IME "সমতা রাশিমালা পড়ার সময় দ্বিমুখী রাশিমালা তৈরি বিফল হয়েছে"
#define PARSER_ERR_IME_FAIL_BINARY_AT_COMPR PARSER_ERR_IME "তুলনামূলক রাশিমালা পড়ার সময় দ্বিমুখী রাশিমালা তৈরি বিফল হয়েছে"
#define PARSER_ERR_IME_FAIL_BINARY_AT_ADDSUB PARSER_ERR_IME "যোগ-বিয়োগ রাশিমালা পড়ার সময় দ্বিমুখী রাশিমালা তৈরি বিফল হয়েছে"
#define PARSER_ERR_INVALID_RIGHT_FACT_EXPR "গুণ-ভাগ-ভাগশেষ রাশিমালা পড়ার সময় দ্বিমুখী রাশিমালার ডান দিক থেকে অবৈধ রাশিমালা পাওয়া গেছে"
#define PARSER_ERR_IME_FAIL_BINARY_AT_MULDIV PARSER_ERR_IME "গুণ-ভাগ-ভাগশেষ রাশিমালা পড়ার সময় দ্বিমুখী রাশিমালা তৈরি বিফল হয়েছে"
#define PARSER_ERR_IME_FAIL_UNARY PARSER_ERR_IME "একমুখী রাশিমালা তৈরি বিফল হয়েছে"
#define PARSER_ERR_INVALID_RIGHT_EXPR_EXPO "ঘাত রাশিমালা পড়ার সময় দ্বিমুখী রাশিমালার ডান দিক থেকে অবৈধ রাশিমালা পাওয়া গেছে"
#define PARSER_ERR_IME_FAIL_BINARY_AT_EXPO PARSER_ERR_IME "ঘাত রাশিমালা পড়ার সময় দ্বিমুখী রাশিমালা তৈরি বিফল হয়েছে"
#define PARSER_ERR_CALL_CANT_TOOMANY_ARGS "কাজ চালানোর রাশিতে ২৫৫-টির বেশি প্রেরণ মান ব্যবহার করা যায় না"


#define PARSER_ERR_EXPECT_RPAREN_ARGS "প্রেরণমানগুলির পরে প্রথম বন্ধনী ')' পাওয়া উচিত ছিল"
#define PARSER_ERR_IME_FAIL_CALL_EXPR PARSER_ERR_IME "কাজ চালানোর রাশি তৈরি বিফল হয়েছে"
#define PARSER_ERR_INVALID_SUBS_IDX "সূচকীয় মানে অবৈধ সূচক"
#define PARSER_ERR_EXPECT_RSBRACK_SUBS "সূচকীয় রাশির পর দ্বিতীয় বন্ধনী ']' পাওয়া উচিত ছিল"
#define PARSER_ERR_IME_FAIL_SUBS_EXPR PARSER_ERR_IME "সূচকীয় মান রাশিমালা তৈরি বিফল হয়েছে"

#define PARSER_ERR_EXPECT_MOD_CHILD "উৎসের সদস্যের নাম পাওয়া উচিত ছিল"
#define PARSER_ERR_IME_FAIL_MOD_CHILD PARSER_ERR_IME "উৎসের সদস্য রাশি তৈরি বিফল হয়েছে"
#define PARSER_ERR_EXPECT_COMMA_ARR_ITEM "তালিকার উপাদানের পর কমা ',' পাওয়া উচিত ছিল"
#define PARSER_ERR_EXPECT_RSBRACK_ARR "তালিকার উপাদানগুলির শেষ দ্বিতীয় বন্ধনী ']' পাওয়া উচিত ছিল"
#define PARSER_ERR_EXPECT_COLON_MAPKEY "ছকের সূচকের পর কোলন ':' পাওয়া উচিত ছিল"
#define PARSER_ERR_EXPECT_COMMA_MAPPAIR "ছকের সূচক-মান যুগলের পর কমা ',' পাওয়া উচিত ছিল"
#define PARSER_ERR_EXPECT_RBRACE_MAP "ছকের পর তৃতীয় বন্ধনী '}' পাওয়া উচিত ছিল"


#define PARSER_ERR_IME_FAIL_BOOL_LIT PARSER_ERR_IME "সত্যি/মিথ্যা রাশি তৈরি বিফল হয়েছে"
#define PARSER_ERR_IME_FAIL_NIL_LIT PARSER_ERR_IME "নিল রাশি তৈরি বিফল হয়েছে"
#define PARSER_ERR_MALFRM_NUM "অবৈধ সংখ্যা পাওয়া গেছে"
#define PARSER_ERR_IME_FAIL_NUM_LIT PARSER_ERR_IME "সংখ্যা রাশি তৈরি বিফল হয়েছে"
#define PARSER_ERR_IME_FAIL_STR_LIT PARSER_ERR_IME "লেখা রাশি তৈরি বিফল হয়েছে"
#define PARSER_ERR_IME_FAIL_IDENT_EXPR PARSER_ERR_IME "চলরাশি তৈরি বিফল হয়েছে"
#define PARSER_ERR_EXPECT_RPAREN_GRP "গুচ্ছ রাশিমালার পর প্রথম বন্ধনী ')' পাওয়া উচিত ছিল"

#define PARSER_ERR_IME_FAIL_GRP PARSER_ERR_IME "গুচ্ছ রাশিমালা তৈরি বিফল হয়েছে"
#define PARSER_ERR_IME_FAIL_ARR PARSER_ERR_IME "তালিকা রাশিমালা তৈরি বিফল হয়েছে"
#define PARSER_ERR_IME_FAIL_MAP PARSER_ERR_IME "ছক রাশিমালা তৈরি বিফল হয়েছে"
#define PARSER_ERR_EXPECT_EXPR "রাশিমালা পাওয়া উচিত ছিল"
#define PARSER_ERR_IME_FAIL_EXPSTMT PARSER_ERR_IME "রাশিমালা বিবৃতি তৈরি বিফল হয়েছে"
#define PARSER_ERR_IME_FAIL_DBGSTMT PARSER_ERR_IME "পরিচিতি বিবৃতি তৈরি বিফল হয়েছে"
#define PARSER_ERR_EXPECT_IDENT_LET "'ধরি'/'let'/'dhori' এর পর চলরাশির নাম পাওয়া উচিত ছিল"
#define PARSER_ERR_EXPECT_EQ_IDENT_LET "চলরাশি তৈরি বিবৃতির চলরাশির নামের পর সমান চিহ্ন '=' পাওয়া উচিত ছিল"
#define PARSER_ERR_IME_FAIL_LETSTMT PARSER_ERR_IME "চলরাশি তৈরি বিবৃতি তৈরি বিফল হয়েছে"
#define PARSER_ERR_EXPECT_RBRACE_BLK PARSER_ERR_IME "বিবৃতি গুচ্ছের পর তৃতীয় বন্ধনী '}' পাওয়া উচিত ছিল"
#define PARSER_ERR_IME_FAIL_BLKSTMT PARSER_ERR_IME "বিবৃতি গুচ্ছ তৈরি বিফল হয়েছে"
#define PARSER_ERR_EXPECT_END_BLK "বিবৃতি গুচ্ছের পর 'শেষ'/'end'/'shesh' পাওয়া উচিত"
#define PARSER_ERR_NOP_ELSE_BLOCK "বিবৃতি গুচ্ছের পর 'নাহলে'/'else'/'nahole' পাওয়া উচিত"
#define PARSER_ERR_NOP_END_BLOCK "বিবৃতি গুচ্ছের পর 'শেষ'/'end'/'shesh' পাওয়া উচিত"
#define PARSER_ERR_EXPECT_THEN_IF "যদি-তাহলে বিবৃতির শর্তের পর 'তাহলে'/'then'/'tahole' পাওয়া উচিত ছিল"

#define PARSER_ERR_IME_FAIL_IFSTMT PARSER_ERR_IME "যদি-তাহলে বিবৃতি তৈরি বিফল হয়েছে"

#define PARSER_ERR_EXPECT_DO_WHILE "যতক্ষণ-করো বিবৃতির শর্তের পর 'করো'/'do'/'koro' পাওয়া উচিত ছিল"
#define PARSER_ERR_IME_FAIL_WHLSTMT PARSER_ERR_IME "যতক্ষণ-করো বিবৃতি তৈরি বিফল হয়েছে"
#define PARSER_ERR_EXPECT_SCOLON_EMT_RET "খালি ফেরাও এর পর একটি সেমিকোলন ';' পাওয়া উচিত ছিল"
#define PARSER_ERR_IME_FAIL_RETSTMT PARSER_ERR_IME "ফেরাও বিবৃতি তৈরি বিফল হয়েছে"
#define PARSER_ERR_IME_FAIL_BRKSTMT PARSER_ERR_IME "ভাঙো বিবৃতি তৈরি বিফল হয়েছে"
#define PARSER_ERR_IME_FAIL_CNTSTMT PARSER_ERR_IME "চালাও বিবৃতি তৈরি বিফল হয়েছে"
#define PARSER_ERR_EXPECT_FN_NAME_FUNC "'কাজ'/'func'/'kaj' এর পর কাজের নাম পাওয়া উচিত ছিল"
#define PARSER_ERR_EXPECT_LPAREN_FN_NAME "কাজের নামের পর প্রথম বন্ধনী '(' পাওয়া উচিত ছিল"
#define PARSER_ERR_EXPECT_PARAM_FNPRM "কাজের নামের পর প্রাপ্তিমান পাওয়া উচিত ছিল"
#define PARSER_ERR_EXPECT_RPAREN_FN_PARM "প্রাপ্তিমানগুলির পর প্রথম বন্ধনী ')' পাওয়া উচিত ছিল"

#define PARSER_ERR_IME_FAIL_FNCSTMT PARSER_ERR_IME "কাজ বিবৃতি তৈরি বিফল হয়েছে"
#define PARSER_ERR_IME_FAIL_IMPSTMT PARSER_ERR_IME "আনয়ন বিবৃতি তৈরি বিফল হয়েছে"
/* clang-format on */
#ifdef __cplusplus
}
#endif

#endif
