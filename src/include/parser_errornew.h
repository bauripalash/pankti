#ifndef PANKTI_PARSER_ERRORS_H
#define PANKTI_PARSER_ERRORS_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
// Internal Error : String
#define PARSER_ERR_IME "অভ্যন্তরীণ গোলমাল: "
// ctx -er por thing paoa uchit chilo
#define PERR_FMT_EXPECT(ctx, thing) ctx "-এর পর " thing " পাওয়া উচিত ছিল"
// Internal Error : thing toiri bifol
#define PERR_FMT_IME(thing) "অভ্যন্তরীণ গোলমাল: " thing " তৈরি বিফল হয়েছে"
// Internal Error : ctx porar somoy thing toiri bifol
#define PERR_FMT_IME_AT(ctx, thing) ("অভ্যন্তরীণ গোলমাল: " ctx " পড়ার সময় " thing " তৈরি বিফল হয়েছে")
#endif
