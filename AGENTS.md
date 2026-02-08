# AGENTS.md - Pankti Programming Language

## Project Overview
**Pankti** (`পঙক্তি`) is a dynamically typed programming language designed for
programming in Bengali, English, Bengali phonetic and combinations for them. It
is implemented primarily in C.

## Pankti Language Syntax Guide
Pankti supports three syntax modes that can be mixed freely.
1. **Bengali**: Native Bengali Script
2. **Phonetic Bengali**: Bengali words written in Latin script.
3. **English**: Standard English keywords

### Keywords
| Concept              | Bengali  | Phonetic   | English  |
|----------------------|----------|------------|----------|
| Variable declaration | `ধরি`    | `dhori`    | `let`    |
| Function declaration | `কাজ`    | `kaj`      | `func`   |
| If condition         | `যদি`    | `jodi`     | `if`     |
| Then                 | `তাহলে`  | `tahole`   | `then`   |
| Else                 | `নাহলে`  | `nahole`   | `else`   |
| While loop           | `যতক্ষণ` | `jotokhon` | `while`  |
| Do (loop body)       | `করো`    | `koro`     | `do`     |
| End block            | `শেষ`    | `sesh`     | `end`    |
| Return               | `ফেরাও`  | `ferao`    | `return` |
| Import module        | `আনয়ন`   | `anoyon`   | `import` |
| Logical And          | `এবং`    | `ebong`    | `and`    |
| Logical Or           | `বা`     | `ba`       | `or`     |
| True                 | `সত্যি`  | `sotti`    | `true`   |
| False                | `মিথ্যা` | `mittha`   | `false`  |
| Nil (null)           | `নিল`    | `nil`      | `nil`    |
| Break                | `ভাঙো`   | `bhango`   | `break`  |
| Length/Size          | `আয়তন`   | `ayoton`   | `len`    |
| Panic/Error          | `গোলযোগ` | `golmal`   | `panic`  |

### Operators

#### Arithmetic

| Operator | Description |
|----------|-------------|
| `+` | Addition |
| `-` | Subtraction |
| `*` | Multiplication |
| `/` | Division |
| `**` | Exponentiation |

#### Comparison
| `==` | Equal |
| `!=` | Not Equal |
| `>`  | Greater Than |
| `>=` | Greater Then or Equal |
| `<`  | Less Than |
| `<=` | Less Than or Equal |

#### Logical
| Bengali | Phonetic | English | Description |
|---------|----------|---------|-------------|
| `এবং` | `ebong` | `and` | Logical AND |
| `বা`  | `ba`    | `or`  | Logical OR |
| `!`  | `!`     | `!`   | Logical NOT |

### Built-in Functions
| Bengali | Phonetic | English | Description | Parameter Count |
|---------|----------|---------|-------------|-----------------|
| `দেখাও(.)`| `dekhao(.)` | `show(.)` | Print values to output| Any numbers |


### Data Types

#### Numbers
Pankti Supports Bengali and Arabic numerals and mixture of them both.

```pankti
১২৩৪৫
12345
৩.১৪
9৯.৯9
```

#### Strings
```pankti
"পলাশ বাউরি"
"Hello world"
```

#### Booleans
```pankti
সত্যি, true, sotti
মিথ্যা, false, mittha
```

#### Arrays
```pankti
["পলাশ", "বাউরি", 3.14, সত্যি]
```

#### Nil
```pankti
nil
নিল
```

#### Maps/Hash tables
```pankti 
{
    "নাম" : "পলাশ",
    "পদবী" : "বাউরি"
    "সময়" : ১২.১২
}
```

### Variable Declaration
```pankti
// Bengali
ধরি নাম = "পলাশ"

// Phonetic
dhori nam = "Palash"

// English
let name = "Palash"
```

### Functions
```pankti
// Bengali
কাজ যোগ(ক, খ)
    ফেরাও ক + খ
শেষ

// Phonetic
kaj jog(a, b)
    ferao a + b
sesh

// English
func add(a, b)
     return a + b
end
```

#### Function Call
```pankti
add(5, 5)
যোগ(৫, ৫)
```

#### If-Then-Else Conditional
```pankti
// Bengali
যদি <শর্ত> তাহলে
    # কিছু কাজ
নাহলে
    # কিছু কাজ
শেষ

// Phonetic
jodi <condition> tahole
     # some work
nahole
    # some work
end

// English
if <condition> then
   # some work
else
    # some work
end
```

#### While Loop
```pankti
// Bengali
যতক্ষণ <শর্ত> করো
    # কিছু কাজ
শেষ

// Phonetic
jotokhon <condition> koro
    # some work
sesh

// English
while <condition> do
    # some work
end
```

#### Subscripting
```pankti
ধরি তালিকা = [১,২,৩]
ধরি তথ্য = { "নাম" : "পলাশ", ১ : ২  }

তালিকা[০] //১
তথ্য["নাম"] // পলাশ
```




