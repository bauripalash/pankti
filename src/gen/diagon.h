#ifndef PANKTI_DIAGON_H
#define PANKTI_DIAGON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#define NO_ERR_CTX_ERR "প্রয়োজনীয় প্রস্তুতি ছাড়াই পঙক্তি চালু হয়েছে"

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
    // অভ্যন্তরীণ গোলমাল: কথা রাশি তৈরি বিফল হয়েছে
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
    PARSER_EXPECT_SEMICOLON,
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
    PARSER_INVALID_ASSIGN,
    // গুণ-ভাগ-ভাগশেষ রাশিমালা পড়ার সময় দ্বিমুখী রাশিমালার ডান দিক থেকে অবৈধ রাশিমালা পাওয়া গেছে
    PARSER_INVALID_MULDIV_RIGHT,
    // ঘাত রাশিমালা পড়ার সময় দ্বিমুখী রাশিমালার ডান দিক থেকে অবৈধ রাশিমালা পাওয়া গেছে
    PARSER_INVALID_EXPO_RIGHT,
    // অবৈধ সংখ্যা পাওয়া গেছে
    PARSER_MALFORMED_NUMBER,
    // উৎস যে নামে আনা হবে তার নাম পাওয়া উচিত ছিল
    PARSER_EXPECT_IMPORT_NAME,
    // %s
    PARSER_TEMPLATE,
    // অভ্যন্তরীণ গোলমাল: স্ট্রিং তৈরি বিফল হয়েছে
    COMPILER_IME_STRING,
    // দ্বিমুখী রাশিমালার বামদিকের রাশিমালা কম্পাইল বিফল হয়েছে
    COMPILER_LEFT_BINARY,
    // দ্বিমুখী রাশিমালার ডানদিকের রাশিমালা কম্পাইল বিফল হয়েছে
    COMPILER_RIGHT_BINARY,
    // যৌক্তিক রাশিমালার বামদিকের রাশিমালা কম্পাইল বিফল হয়েছে
    COMPILER_LEFT_LOGICAL,
    // যৌক্তিক রাশিমালার ডানদিকের রাশিমালা কম্পাইল বিফল হয়েছে
    COMPILER_RIGHT_LOGICAL,
    // একমুখী রাশিমালা কম্পাইল বিফল হয়েছে
    COMPILER_UNARY_EXPR,
    // তালিকার উপাদান রাশি কম্পাইল বিফল হয়েছে
    COMPILER_ARRAY_ITEM,
    // ছকের সূচক রাশি কম্পাইল বিফল হয়েছে
    COMPILER_MAP_KEY,
    // ছকের মান রাশি কম্পাইল বিফল হয়েছে
    COMPILER_MAP_VALUE,
    // প্রাথমিক স্তরের বিবৃতি কম্পাইল বিফল হয়েছে
    COMPILER_TOP_STMT,
    // মান নির্ধারণ রাশিমালার মান কম্পাইল বিফল হয়েছে
    COMPILER_ASSIGN_VALUE,
    // মান নির্ধারণের রাশিমালার সূচকীয় রাশির সূচক কম্পাইল বিফল হয়েছে
    COMPILER_SUB_ASSIGN_SUBINDEX,
    // মান নির্ধারণ রাশিমালার সূচকীয় রাশির মুল মান কম্পাইল বিফল হয়েছে
    COMPILER_SUB_ASSIGN_SUBVALUE,
    // সূচকীয় রাশির সূচক কম্পাইল বিফল হয়েছে
    COMPILER_SUBS_INDEX,
    // সূচকীয় রাশির মুল মান কম্পাইল বিফল হয়েছে
    COMPILER_SUBS_VALUE,
    // মান নির্ধারণ রাশিমালার সূচকীয় রাশির নতুন মান কম্পাইল বিফল হয়েছে
    COMPILER_SUB_ASSIGN_VALUE,
    // কাজের ডাক রাশিমালার কাজের নাম রাশিমালা কম্পাইল বিফল হয়েছে
    COMPILER_CALL_EXPR_CALLE,
    // কাজের ডাক রাশিমালার প্রেরণমান রাশিমালা কম্পাইল বিফল হয়েছে
    COMPILER_CALL_EXPR_ARG,
    // উৎসের সদস্য ব্যবহারের রাশিমালা কম্পাইল বিফল হয়েছে
    COMPILER_MOD_EXPR,
    // রাশিমালা বিবৃতি কম্পাইল বিফল হয়েছে
    COMPILER_EXPR_STMT,
    // পরিচিতি বিবৃতি কম্পাইল বিফল হয়েছে
    COMPILER_DEBUG_STMT,
    // চলরাশি তৈরির বিবৃতি কম্পাইল বিফল হয়েছে
    COMPILER_LET_STMT,
    // বিবৃতি গুচ্ছ কম্পাইল বিফল হয়েছে
    COMPILER_BLOCK_STMT,
    // যদি-তাহলে বিবৃতির শর্ত রাশিমালা কম্পাইল বিফল হয়েছে
    COMPILER_IF_COND,
    // যদি-তাহলে বিবৃতির তাহলে -র বিবৃতি গুচ্ছ কম্পাইল বিফল হয়েছে
    COMPILER_IF_THEN_BLOCK,
    // যদি-তাহলে বিবৃতির নাহলে -র বিবৃতি গুচ্ছ কম্পাইল বিফল হয়েছে
    COMPILER_IF_ELSE_BLOCK,
    // যতক্ষণ-করো বিবৃতির শর্ত রাশিমালা কম্পাইল বিফল হয়েছে
    COMPILER_WHILE_COND,
    // যতক্ষণ-করো বিবৃতির বিবৃতিগুচ্ছ কম্পাইল বিফল হয়েছে
    COMPILER_WHILE_BLOCK,
    // কাজের বিবৃতিগুচ্ছ কম্পাইল বিফল হয়েছে
    COMPILER_FUNC_BLOCK,
    // কাজ বিবৃতি কম্পাইল বিফল হয়েছে
    COMPILER_FUNC_STMT,
    // ফেরাও বিবৃতির রাশিমালা কম্পাইল বিফল হয়েছে
    COMPILER_RETURN_STMT,
    // উৎস আনয়ন বিবৃতি কম্পাইল বিফল হয়েছে
    COMPILER_MODULE_STMT,
    // ভাঙো বিবৃতি কম্পাইল বিফল হয়েছে
    COMPILER_BREAK_STMT,
    // চালাও বিবৃতি কম্পাইল বিফল হয়েছে
    COMPILER_CONTINUE_STMT,
    // শর্তজাতিয় বিবৃতি গুচ্ছের সংখ্যা অনেক বেশি
    COMPILER_COND_JUMP_BIG,
    // চলরাশি তৈরির সময় চলরাশির মান সেই নামের চলরাশির মান হতে পারে না
    COMPILER_VAR_OWN_INIT,
    // অবৈধ রাশিমালা পাওয়া গেছে
    COMPILER_INVALID_EXPR,
    // '%s' নামের চলরাশি এইখানে আগের থেকেই আছে 
    COMPILER_VAR_EXISTS,
    // অনেক বেশি স্থানীয় চলরাশি পাওয়া গেছে
    COMPILER_LOCAL_TOO_MANY,
    // অনেক বেশি স্থানীয় এবং নিকটস্থ স্থানীয় চলরাশি পাওয়া গেছে
    COMPILER_CLOSURE_TOO_MANY,
    // যতক্ষণ-করো বিবৃতি কম্পাইল করার জন্য কিছু প্রয়োজন অভ্যন্তরীণ তথ্য তৈরি বিফল হয়েছে
    COMPILER_WHILE_BLOCK_CTX,
    // প্রাথমিক স্তরে ফেরাও বিবৃতি ব্যবহার করা যায় না
    COMPILER_RETURN_TOP_LEVEL,
    // অভ্যন্তরীণ গোলমাল: প্রয়োজনীয় প্রস্তুতি ছাড়াই পঙক্তি চালু হয়েছে
    COMPILER_IME_NOCTX,
    // উৎস বিবৃতির উৎসের পথ কম্পাইল বিফল হয়েছে
    COMPILER_IMPORT_PATH,
    // চলরাশির নাম কম্পাইল বিফল হয়েছে
    COMPILER_IDENT_NAME,
    // %s
    COMPILER_TEMPLATE,
    // কথা রাশির মধ্যে অজানা বিশেষ চিহ্ন পাওয়া গেছে
    STR_UNKNOWN_ESCAPE,
    // কথা রাশির মধ্যে বিশেষ চিহ্নে অবৈধ হেক্স অক্ষর পাওয়া গেছে
    STR_INVALID_HEX,
    // অভ্যন্তরীণ গোলমাল: বিশেষ চিহ্নযুক্ত কথা রাশি তৈরি বিফল হয়েছে
    STR_IME_ESCAPED_STR,
    // কথা রাশির শেষে অসম্পূর্ণ বিশেষ চিহ্ন পাওয়া গেছে
    STR_INCOMPLETE_ESCAPE_STR,
    // \\uHHHH -জাতিয় বিশেষ চিহ্ন পড়ার সময় তার low-surrogate \\uHHHH চিহ্ন আশা করা হয়েছিল কিন্তু তা পাওয়া যায়নি
    STR_NO_LOW_SURROGATE,
    // \\uHHHH -জাতিয় বিশেষ চিহ্নে শুধুমাত্র একলা low-surrogate পাওয়া গেছে
    STR_LONE_LOW_SURROGATE,
    // অবৈধ \\uHHHH -জাতিয় low-surrogate চিহ্ন পাওয়া গেছে
    STR_INVALID_LOW_SURROGATE,
    // কথার মধ্যে অবৈধ \\uHHHHHHHH -জাতিয় বিশেষ চিহ্ন পাওয়া গেছে
    STR_INVALID_H8,
    // অভ্যন্তরীণ গোলমাল: বিশেষ চিহ্নযুক্ত কথা রাশির জন্য প্রয়োজনীয় মেমোরি পাওয়া যায়নি
    STR_IME_ESCAPED_OUTPUT,
    // অভ্যন্তরীণ গোলমাল: পঙক্তি স্ক্রিপ্ট চালানোর প্রাথমিক প্রস্তুতি বিফল হয়েছে
    RT_IME_PRIMARY_SETUP,
    // শুন্য দিয়ে ভাগ করা সম্ভব নয়
    RT_DIV_ZERO,
    // পঙক্তি অভ্যন্তরীণ গোলযোগঃ স্ট্যাক ওভারফ্লো
    RT_STACK_OVERFLOW,
    // পঙক্তি অভ্যন্তরীণ গোলযোগঃ স্ট্যাক আন্ডারফ্লো
    RT_STACK_UNDERFLOW,
    // দ্বিমুখী রাশিমালার মান গণনা বিফল হয়েছে
    RT_BINARY_OP,
    // কথা রাশিযুক্ত দ্বিমুখী রাশিমালার মান গণনা বিফল হয়েছে
    RT_STR_BINARY_OP,
    // অবৈধ দ্বিমুখী রাশিমালা পাওয়া গেছে
    RT_INVALID_BINARY_OP,
    // অবৈধ তুলনামূলক রাশিমালা পাওয়া গেছে
    RT_INVALID_COMP_OP,
    // কাজের ডাকের প্রেরণমান আর কাজের প্রাপ্তিমানের সংখ্যা সমান নয়ঃ কাজের জন্য %llu-টি প্রেরণমান দরকার ছিল কিন্তু %llu-টি দেওয়া হয়েছে
    RT_ARG_PARAM_NOTEQ,
    //  %s সাধারণ কাজের ডাকের প্রেরণমান আর কাজের প্রাপ্তিমানের সংখ্যা সমান নয়ঃ কাজের জন্য %llu-টি প্রেরণমান দরকার ছিল কিন্তু %llu-টি দেওয়া হয়েছে
    RT_NATIVE_ARG_PARAM_NOTEQ,
    // তালিকার জন্য সূচক সংখ্যা হওয়া উচিত ছিল কিন্তু প্রাপ্ত সূচকের ধরন হল %s
    RT_INVALID_ARR_INDEX_NOTNUM,
    // তালিকার জন্য সূচক ধনাত্মক পূর্ণসংখ্যা হওয়া উচিত ছিল কিন্তু প্রাপ্ত সূচক হল '%f'
    RT_INVALID_ARR_INDEX,
    // তালিকার সূচকের মান তালিকার উপাদান সংখ্যার বেশি, এই তালিকার বৈধ সূচক হল ০ থেকে %llu কিন্তু দেওয়া হয়েছে %llu
    RT_ARR_INDEX_OUT_RANGE,
    // '%s'-জাতিয় মান ছকের সূচক হিসাবে ব্যবহার সম্ভব নয়
    RT_INVALID_MAP_KEY,
    // ছকের মধ্যে সূচকের মান খুঁজে পাওয়া গেল না
    RT_MAP_KEY_NOT_FOUND,
    // %s-জাতিয় রাশির সূচকীয় মান ব্যবহার সম্ভব নয়
    RT_INVALID_SUBS_TARGET,
    // %s-জাতিয় রাশির সূচকীয় মান নির্ধারণ সম্ভব নয়
    RT_INVALID_SUBASSIGN,
    // উৎস আনয়নের উৎসের স্থান একটি কথা রাশি হওয়া উচিত ছিল কিন্তু তার বদলে একটি %s পাওয়া গেল
    RT_INVALID_IMPORT_PATH,
    // বর্তমানে শুধুমাত্র সাধারণ উৎসগুলি ব্যবহার করা যায়
    RT_ONLY_STDLIB,
    // অভ্যন্তরীণ গোলমাল: উৎস আনয়নের জন্য প্রয়োজনীয় প্রস্তুতি বিফল হয়েছে
    RT_IME_MODULE,
    // অভ্যন্তরীণ গোলমাল: স্থানীয় মান ব্যবহারের জন্য প্রয়োজনীয় প্রস্তুতি বিফল হয়েছে
    RT_IME_CLOSURE,
    // শুধুমাত্র চলরাশির মান নির্ধারণ করা যায়, কিন্তু একটি %s-জাতিয় রাশি পাওয়া গেছে
    RT_INVALID_VAR_DECLARE,
    // %s নামের কোনো চলরাশি খুঁজে পাওয়া গেল না
    RT_UNDEF_GET_VAR,
    // %s নামের চলরাশির মান নির্ধারণ সম্ভব নয়, এই নামের কোনো চলরাশি খুঁজে পাওয়া গেল না
    RT_UNDEF_SET_VAR,
    // কাজের ডাক বিফল হয়েছে
    RT_CALL_FAIL,
    // শুধুমাত্র কাজকেই কাজের ডাকে ব্যবহার করা যায় কিন্তু একটি %s-জাতিয় রাশি পাওয়া গেছে
    RT_INVALID_CALLEE,
    // প্রচুর পরিমাণ কাজের ডাক একইসঙ্গে ব্যবহৃত হচ্ছে
    RT_CALLSTACK_OVERFLOW,
    // শুধুমাত্র কাজের ক্ষেত্রেই স্থানীয় ও প্রতিবেশী চলরাশি প্রস্তুতি নেওয়া সম্ভব কিন্তু একটি %s-জাতিয় রাশি পাওয়া গেছে
    RT_ONLY_FUNC_CLOSURE,
    // অভ্যন্তরীণ গোলমাল: তালিকা তৈরি বিফল হয়েছে
    RT_IME_ARRAY,
    // অভ্যন্তরীণ গোলমাল: ছক তৈরি বিফল হয়েছে
    RT_IME_MAP,
    // শুধুমাত্র উৎস থেকেই সদস্য পাওয়া যায় কিন্তু একটি %s-জাতিয় রাশি থেকে সদস্য নেওয়ার চেষ্টা করা হয়েছে
    RT_ONLY_MOD_CHILD,
    // উৎসের থেকে অবৈধ সদস্য ব্যবহারের চেষ্টা করা হয়েছে %s-জাতিয় রাশি পাওয়া গেছে
    RT_INVALID_MOD_CHILD,
    // অজানা উৎস %s পাওয়া গেছে
    RT_UNKNOWN_MOD,
    // উৎসের অজানা সদস্য পাওয়া গেছে
    RT_UNKNOWN_CHILD,
    // %s
    RT_TEMPLATE,

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
