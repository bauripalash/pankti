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
    "ধরি" "dhori" "let"
    "কাজ" "kaj" "func"
    "যদি" "jodi" "if"
    "তাহলে" "tahole" "then"
    "নাহলে" "nahole" "else"
    "যতক্ষণ" "jotokhon" "while"
    "করো" "koro" "do"
    "শেষ" "sesh" "end"
    "ফেরাও" "ferao" "return"
    "আনয়ন" "anoyon" "import"
    "এবং" "ebong" "and"
    "বা" "ba" "or"
    "সত্যি" "sotti" "true"
    "মিথ্যা" "mittha" "false"
    "নিল" "nil"
    "ভাঙো" "bhango" "break"
    "গোলমাল" "golmal" "panic"
    )

    '(;; Built-in Function (Bengali)
      ("\\<\\(দেখাও\\|আয়তন\\|সংযোগ\\|সময়\\)\\s-*(" 1 font-lock-builtin-face)
      ;; Built-in Function (Phonetic)
      ("\\<\\(dekhao\\|ayoton\\|songjog\\|somoy\\)\\s-*(" 1 font-lock-builtin-face)
      ;; Built-in Function (English)
      ("\\<\\(show\\|len\\|append\\|clock\\)\\s-*(" 1 font-lock-builtin-face)

      ;; Function definitions
      ("\\<(কাজ\\|kaj\\|func\\)\\s-+\\([[:alnum:]_\u0980-\u09FF]+\\)" 2 font-lock-function-name-face)

      ;; Variable declarations
      ("\\<(ধরি\\|dhori\\|flet\\)\\s-+\\([[:alnum:]_\u0980-\u09FF]+\\)" 2 font-lock-variable-name-face)

      ;; Numbers
      ("\\<[০-৯0-9]+\\(?:\\.[০-৯0-9]+\\)?\\>" . font-lock-number-face)

      ;; Operators
      ("\\(\\*\\*\\|==\\|!=\\|>=\\|<=\\|[+\\-*/><]\\)" . font-lock-operator)

      ;; String literals
      ("\"[^\"]*\"" . font-lock-string-face))

    '("\\.pn\\'" "\\.pank\\'")

    nil

    "Syntax Highlighting for Pankti Programming Language"
)

(provide 'pankti-emacs-mode)
