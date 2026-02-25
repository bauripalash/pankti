// Define keyword lists
var keywords_en = 'let|func|if|then|else|end|while|do|return|import|panic|break|len|and|or';
var keywords_pn = 'dhori|kaj|jodi|tahole|nahole|sesh|jotokhon|koro|ferao|anoyon|golmal|bhango|ayoton|ebong|ba';
var keywords_bn = 'ধরি|কাজ|যদি|তাহলে|নাহলে|শেষ|যতক্ষণ|করো|ফেরাও|আনয়ন|গোলমাল|ভাঙো|আয়তন|এবং|বা';

var booleans_en = 'true|false';
var booleans_pn = 'sotti|mittha';
var booleans_bn = 'সত্যি|মিথ্যা';

var nil_en = 'nil';
var nil_bn = 'নিল';

var builtins_en = 'show|len|append|clock';
var builtins_pn = 'dekhao|ayoton|songjog|somoy';
var builtins_bn = 'দেখাও|আয়তন|সংযোগ|সময়';

Prism.languages.pankti = {
'comment': {
  pattern: /\/\/.*/,
  greedy: true
},
'string': {
  pattern: /"(?:[^"\\]|\\.)*"/,
  greedy: true
},
'boolean': {
  pattern: new RegExp('\\b(?:' + booleans_en + '|' + booleans_pn + ')\\b|(?:' + booleans_bn + ')(?![\\u0980-\\u09FF])'),
  greedy: true
},
'nil': {
  pattern: new RegExp('\\b(?:' + nil_en + ')\\b|(?:' + nil_bn + ')(?![\\u0980-\\u09FF])'),
  greedy: true
},
'keyword': {
  pattern: new RegExp('\\b(?:' + keywords_en + '|' + keywords_pn + ')\\b|(?:' + keywords_bn + ')(?![\\u0980-\\u09FF])'),
  greedy: true
},

'builtin': {
    pattern: new RegExp(
        '(?:\\b(?:' + builtins_en + '|' + builtins_pn + ')\\b|(?:' + builtins_bn + ')(?![\\u0980-\\u09FF]))(?=\\s*\\()'
    ),
    greedy: true
},
'function': {
  pattern: /[a-zA-Z_\u0980-\u09FF][a-zA-Z0-9_\u0980-\u09FF]*(?=\s*\()/,
  greedy: true
},
'number': [
  { pattern: /[০-৯]+(?:\.[০-৯]+)?/, greedy: true },
  { pattern: /\b\d+(?:\.\d+)?\b/, greedy: true }
],
'operator': /\*\*|[=!<>]=?|[+\-*/%]/,
'punctuation': /[{}[\]();,:]/
};

// Remove 'variable' entirely - unmatched text stays as plain text
// This prevents keywords from being overwritten

Prism.languages.pn = Prism.languages.pankti;
Prism.languages.pank = Prism.languages.pankti;
