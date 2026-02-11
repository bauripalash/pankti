;;; -*- lexical-binding: t; -*-
;;; Syntax Highlighting for Pankti Programming Language
;;; pankti-emacs-mode.el 

;; Author: Palash Bauri
;; Version: 0.1.0

(require 'generic-x)

(define-generic-mode 
  'pankti-emacs-mode
  ;; Comments
  '("//")

  ;; Keywords (Bengali, Bengali Phonetic, English)
  '(
    "dhori" "let"
    "kaj" "func"
    "jodi" "if"
    "tahole" "then"
    "nahole" "else"
    "jotokhon" "while"
    "koro" "do"
    "sesh" "end"
    "ferao" "return"
    "anoyon" "import"
    "ebong" "and"
    "ba" "or"
    "sotti" "true"
    "mittha" "false"
    "nil"
    "bhango" "break"
    "golmal" "panic"
    )

  '(
    ("ধরি\\|কাজ\\|যদি\\|তাহলে\\|নাহলে\\|যতক্ষণ\\|করো\\|শেষ\\|ফেরাও\\|আনয়ন\\|এবং\\|বা\\|সত্যি\\|মিথ্যা\\|নিল\\|ভাঙো\\|গোলমাল" . font-lock-keyword-face)
    ;; Built-in Function (Bengali)
      ("\\(দেখাও\\|আয়তন\\|সংযোগ\\|সময়\\)\\s-*(" 1 font-lock-builtin-face)
      ;; Built-in Function (Phonetic)
      ("\\(dekhao\\|ayoton\\|songjog\\|somoy\\)\\s-*(" 1 font-lock-builtin-face)
      ;; Built-in Function (English)
      ("\\(show\\|len\\|append\\|clock\\)\\s-*(" . font-lock-builtin-face)

      ;; Function definitions
      ("\\(কাজ\\|kaj\\|func\\)\\s-+\\([[:alnum:]_\u0980-\u09FF]+\\)" 2 font-lock-function-name-face)

      ;; Variable declarations
      ("\\(ধরি\\|dhori\\|flet\\)\\s-+\\([[:alnum:]_\u0980-\u09FF]+\\)" 2 font-lock-variable-name-face)

      ;; Numbers
      ("[0-9০-৯]+\\(\\.[0-9০-৯]+\\)?" . font-lock-constant-face)

      ;; Operators
      ("\\(\\*\\*\\|==\\|!=\\|>=\\|<=\\|[+\\-*/><]\\)" . font-lock-operator-face)

      ;; String literals
      ("\"[^\"]*\"" . font-lock-string-face))

    '("\\.pn\\'" "\\.pank\\'")

    nil

    "Syntax Highlighting for Pankti Programming Language"
)

(provide 'pankti-emacs-mode)
