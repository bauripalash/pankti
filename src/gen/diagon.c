#include "diagon.h"
#include <stdbool.h>
#include <stddef.h>
static const PanDiagInfo diagList[PANDIAG_CODE_COUNT] = {
    {LEXER_INVALID_CHAR, PAN_DIAG_LEXER, PAN_DIAG_SEV_ERROR, true, "অপরিচিত অক্ষর পাওয়া গেছে `%s`", ""},
    {LEXER_INVALID_CHAR_AT, PAN_DIAG_LEXER, PAN_DIAG_SEV_ERROR, true, "অপরিচিত অক্ষর পাওয়া গেছে এই অবস্থানেঃ %llu নং লাইনে %llu নং স্তম্ভে", ""},
    {PARSER_IME_LOGICAL_EXPR, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "অভ্যন্তরীণ গোলমাল: যৌক্তিক রাশিমালা তৈরি বিফল হয়েছে", ""},
    {PARSER_IME_ASSIGN_EXPR, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "অভ্যন্তরীণ গোলমাল: মান নির্ধারণ রাশিমালা তৈরি বিফল হয়েছে", ""},
    {PARSER_IME_BINARY_EXPR, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "অভ্যন্তরীণ গোলমাল: দ্বিমুখী রাশিমালা তৈরি বিফল হয়েছে", ""},
    {PARSER_IME_UNARY_EXPR, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "অভ্যন্তরীণ গোলমাল: একমুখী রাশিমালা তৈরি বিফল হয়েছে", ""},
    {PARSER_IME_SUBSCRIPT_EXPR, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "অভ্যন্তরীণ গোলমাল: সূচকীয় রাশিমালা তৈরি বিফল হয়েছে", ""},
    {PARSER_IME_BOOL_EXPR, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "অভ্যন্তরীণ গোলমাল: সত্যমান রাশি তৈরি বিফল হয়েছে", ""},
    {PARSER_IME_NIL_EXPR, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "অভ্যন্তরীণ গোলমাল: নিল রাশি তৈরি বিফল হয়েছে", ""},
    {PARSER_IME_NUM_EXPR, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "অভ্যন্তরীণ গোলমাল: সংখ্যা রাশি তৈরি বিফল হয়েছে", ""},
    {PARSER_IME_STR_EXPR, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "অভ্যন্তরীণ গোলমাল: লেখা রাশি তৈরি বিফল হয়েছে", ""},
    {PARSER_IME_IDENT_EXPR, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "অভ্যন্তরীণ গোলমাল: চলরাশি তৈরি বিফল হয়েছে", ""},
    {PARSER_IME_GROUP_EXPR, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "অভ্যন্তরীণ গোলমাল: গুচ্ছ রাশিমালা তৈরি বিফল হয়েছে", ""},
    {PARSER_IME_ARRAY_EXPR, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "অভ্যন্তরীণ গোলমাল: তালিকা রাশিমালা তৈরি বিফল হয়েছে", ""},
    {PARSER_IME_MAP_EXPR, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "অভ্যন্তরীণ গোলমাল: ছক রাইমালা তৈরি বিফল হয়েছে", ""},
    {PARSER_IME_CALL_EXPR, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "অভ্যন্তরীণ গোলমাল: কাজের ডাক রাশিমালা তৈরি বিফল হয়েছে", ""},
    {PARSER_IME_RETURN_STMT, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "অভ্যন্তরীণ গোলমাল: ফেরাও বিবৃতি তৈরি বিফল হয়েছে", ""},
    {PARSER_IME_BREAK_STMT, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "অভ্যন্তরীণ গোলমাল: ভাঙো বিবৃতি তৈরি বিফল হয়েছে", ""},
    {PARSER_IME_CONTINUE_STMT, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "অভ্যন্তরীণ গোলমাল: চালাও বিবৃতি তৈরি বিফল হয়েছে", ""},
    {PARSER_IME_FUNC_STMT, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "অভ্যন্তরীণ গোলমাল: কাজ বিবৃতি তৈরি বিফল হয়েছে", ""},
    {PARSER_IME_IMPORT_STMT, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "অভ্যন্তরীণ গোলমাল: আনয়ন বিবৃতি তৈরি বিফল হয়েছে", ""},
    {PARSER_IME_IF_STMT, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "অভ্যন্তরীণ গোলমাল: যদি-তাহলে বিবৃতি তৈরি বিফল হয়েছে", ""},
    {PARSER_IME_WHILE_STMT, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "অভ্যন্তরীণ গোলমাল: যতক্ষণ-করো বিবৃতি তৈরি বিফল হয়েছে", ""},
    {PARSER_IME_LET_STMT, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "অভ্যন্তরীণ গোলমাল: চলরাশি তৈরি বিবৃতি তৈরি বিফল হয়েছে", ""},
    {PARSER_IME_EXPR_STMT, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "অভ্যন্তরীণ গোলমাল: রাশিমালা বিবৃতি তৈরি বিফল হয়েছে", ""},
    {PARSER_IME_DEBUG_STMT, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "অভ্যন্তরীণ গোলমাল: পরিচিতি বিবৃতি তৈরি বিফল হয়েছে", ""},
    {PARSER_IME_BLOCK_STMT, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "অভ্যন্তরীণ গোলমাল: বিবৃতি গুচ্ছ তৈরি বিফল হয়েছে", ""},
    {PARSER_IME_MOD_CHILD, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "অভ্যন্তরীণ গোলমাল: উৎসের সদস্য রাশি তৈরি বিফল হয়েছে", ""},
    {PARSER_EXPECT_EXPR, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "রাশিমালা পাওয়া উচিত ছিল", ""},
    {PARSER_EXPECT_SEMICOLOR, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "সেমিকোলন ';' পাওয়া উচিত ছিল", ""},
    {PARSER_EXPECT_FUNC_NAME, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "'কাজ'/'func'/'kaj' এর পরে কাজের নাম পাওয়া উচিত ছিল", "এইভাবে কাজ তৈরি করতে হয় - কাজ কাজের_নাম()..."},
    {PARSER_EXPECT_LPAREN_FUNC_NAME, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "কাজের নামের পর একটি প্রথম বন্ধনী '(' পাওয়া উচিত ছিল", "এইভাবে কাজ তৈরি করতে হয় - কাজ কাজের_নাম()..."},
    {PARSER_EXPECT_FUNC_PARAM, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "কাজের নামের পর কাজের প্রাপ্তিমান পাওয়া উচিত ছিল", "প্রাপ্তিমান যুক্ত কাজ এইভাবে তৈরি করতে হয় - কাজ কাজের_নাম(প্রাপ্তিমান_১,প্রাপ্তি_মান_২)..."},
    {PARSER_EXPECT_FUNC_RPAREN, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "কাজের প্রাপ্তিমানগুলির পর (যদি থাকে) একটি প্রথম বন্ধনী ')' পাওয়া উচিত ছিল", "এইভাবে কাজ তৈরি করতে হয় - কাজ কাজের_নাম()... অথবা কাজ কাজের_নাম(প্রাপ্তিমানগুলি)"},
    {PARSER_EXPECT_CALL_RPAREN, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "কাজের ডাক রাশিমালাতে প্রেরণমানগুলির পর একটি প্রথম বন্ধনী ')' পাওয়া উচিত ছিল", "কাজের ডাক এইভাবে লিখতে হয়, যদি প্রেরণমান থাকে - কাজের_নাম(প্রেরণমানগুলি) কিংবা যদি প্রেরণমান না থাকে কাজের_নাম()"},
    {PARSER_EXPECT_LET_IDENT, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "'ধরি'/'let'/'dhor' -এর পর চলরাশির নাম পাওয়া উচিত ছিল", "এইভাবে নতুন চলরাশি তৈরি করতে হয় - ধরি চলরাশির_নাম = ..."},
    {PARSER_EXPECT_EQ_LET_IDENT, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "চলরাশি তৈরির সময় চলরাশির নামের পর একটি সমান চিহ্ন '=' পাওয়া উচিত ছিল", "এইভাবে নতুন চলরাশি তৈরি করতে হয় - ধরি চলরাশির_নাম = ..."},
    {PARSER_EXPECT_THEN_IFCOND, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "যদি-তাহলে বিবৃতির শর্তের পর 'তাহলে'/'then'/'tahole' পাওয়া উচিত ছিল", "যদি-তাহলে বিবৃতি এইভাবে লিখতে হয় - যদি <শর্ত> তাহলে...শেষ"},
    {PARSER_EXPECT_END_BLOCK, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "বিবৃতি গুচ্ছের পর 'শেষ'/'end'/'shesh' পাওয়া উচিত", ""},
    {PARSER_EXPECT_NOOP_ELSE_BLOCK, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "বিবৃতি গুচ্ছের পর 'নাহলে'/'else'/'nahole' পাওয়া উচিত", ""},
    {PARSER_EXPECT_NOOP_END_BLOCK, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "বিবৃতি গুচ্ছের পর 'শেষ'/'end'/'shesh' পাওয়া উচিত", ""},
    {PARSER_EXPECT_RBRACE_BLOCK, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "বিবৃতি গুচ্ছের পর তৃতীয় বন্ধনী '}' পাওয়া উচিত ছিল", ""},
    {PARSER_EXPECT_DO_WHILECOND, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "যতক্ষণ-করো বিবৃতির শর্তের পর 'করো'/'do'/'koro' পাওয়া উচিত ছিল", "যতক্ষণ-করো বিবৃতি এইভাবে লিখতে হয় - যতক্ষণ <শর্ত> করো...শেষ"},
    {PARSER_EXPECT_SEMICOLON_NILRETURN, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "খালি ফেরাও এর পর একটি সেমিকোলন ';' পাওয়া উচিত ছিল", ""},
    {PARSER_EXPECT_RPAREN_GROUP, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "গুচ্ছ রাশিমালার পর প্রথম বন্ধনী ')' পাওয়া উচিত ছিল", "গুচ্ছ রাশি এইভাবে লিখতে হয় - (রাশি, রাশি, রাশি)"},
    {PARSER_EXPECT_RBRACE_MAP, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "ছকের সূচক-মান যুগলগুলির শেষে তৃতীয় বন্ধনী '}' পাওয়া উচিত ছিল", "ছক এইভাবে লিখতে হয় - {সূচক : মান}"},
    {PARSER_EXPECT_COMMA_MAPPAIR, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "ছকের সূচক-মান যুগলের পর কমা ',' পাওয়া উচিত ছিল", "ছক এইভাবে লিখতে হয় - {সূচক : মান, সূচক : মান}"},
    {PARSER_EXPECT_COLON_MAPKEY, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "ছকের সূচকের পর কোলন ':' পাওয়া উচিত ছিল", "ছক এইভাবে লিখতে হয় - {সূচক : মান}"},
    {PARSER_EXPECT_RBRACKET_ARRAY, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "তালিকার উপাদানগুলির শেষে দ্বিতীয় বন্ধনী ']' পাওয়া উচিত ছিল", "তালিকা এইভাবে লিখতে হয় - [১, ২, ৩, ৪]"},
    {PARSER_EXPECT_COMMA_ARRAYITEM, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "তালিকার উপাদানের পর কমা ',' পাওয়া উচিত ছিল", "তালিকা এইভাবে লিখতে হয় - [১, ২, ৩, ৪]"},
    {PARSER_EXPECT_MOD_CHILD, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "উৎসের সদস্যের নাম পাওয়া উচিত ছিল", ""},
    {PARSER_EXPECT_RBRACKET_SUBEXPR, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "সূচকীয় রাশির পর দ্বিতীয় বন্ধনী ']' পাওয়া উচিত ছিল", ""},
    {PARSER_INVALID_SUBEXPR_INDEX, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "সূচকীয় মানে অবৈধ সূচক", ""},
    {PARSER_CALL_TOOMANY_ARGS, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "কাজ চালানোর রাশিমালাতে ২৫৫-টির বেশি প্রেরণ মান ব্যবহার করা যায় না", ""},
    {PARSER_EXPECT_INVALID_ASSIGN, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "অবৈধ মান নির্ধারণ রাশিমালা, শুধুমাত্র চলরাশি এবং সূচকীয় রাশির মান নির্ধারণ করা যায়", ""},
    {PARSER_INVALID_EXPO_RIGHT, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "ঘাত রাশিমালা পড়ার সময় দ্বিমুখী রাশিমালার ডান দিক থেকে অবৈধ রাশিমালা পাওয়া গেছে", ""},
    {PARSER_MALFORMED_NUMBER, PAN_DIAG_PARSER, PAN_DIAG_SEV_ERROR, false, "অবৈধ সংখ্যা পাওয়া গেছে", ""},

};

const PanDiagInfo *DiagGetInfo(PanDiagCode code) {
    if (code < 0 || code > PANDIAG_CODE_COUNT) {
        return NULL;
    }

    return &diagList[code];
}
const char *DiagGetMsg(PanDiagCode code) { return DiagGetInfo(code)->msg; }

const char *DiagGetHint(PanDiagCode code) {
    const PanDiagInfo *info = DiagGetInfo(code);
    if (info && info->hint) {
        return info->hint;
    } else {
        return NULL;
    }
}
