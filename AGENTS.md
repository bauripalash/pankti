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

| Operator | Description |
|----------|-------------|
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
| Bengali | Phonetic | English | Description | Arguments |
|---------|----------|---------|-------------|-----------|
| `দেখাও()` | `dekhao()` | `show()` | Print values to output| Any numbers |
| `আয়তন()` | `ayoton()` | `len()` | Get length of Array, Strings etc. | 1 |
| `সংযোগ()` | `songjog()` | `append()`| Append value to array | 2 or more |
| `সময়()` | `somoy()` | `clock()` | Get current time | 0 |


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

### Comments
```pankti
// Single line comment
```
### File Extension
Pankti source files use `.pn` extension. `.pank` are also valid for legacy purpose.


## Architecture Overview
### Source Tree Structure
```
src/              
    include/
        gfxfont.h           # Raylib+kb_text_shaping integration
        parser_errors.h     # Parser errors
        token.h             # Token definitions
        interpreter.h       # Tree-Walking Interpreter (Deprecated)
        ansicolors.h        # Color codes for Terminal
        version.h           # Version definition
        parser.h            # Parser
        gfxhelper.h         # Graphics Helper/Utilities for Raylib
        ast.h               # Abstract Syntax Tree definitions
        symtable.h          # Symbol Table for Compiler/VM
        keywords.h          # Keyword definitions (Bengali, English, Phonetic)
        strescape.h         # Helper to escape characters like \n, \t \xHH, \uHHHH etc in strings
        utils.h             # Utilities
        bengali.h           # Bengali characters handling
        lexer.h             # Lexer interface
        core.h              # Core runtime
        defaults.h          # Default values used across the project
        system.h            # OS Detection and Other OS related utilities
        printer.h           # Writing to output device interface
        ustring.h           # Codepoint Iterator interface
        env.h               # Enviornment for Tree-Walking Interpreter (Deprecated)
        pstdlib.h           # Pankti Standard Library definition
        vm.h                # Virtual Machine 
        ptypes.h            # Type definitions (u8, u16, u32, u64)
        compiler.h          # Bytecode Compiler
        gc.h                # Garbage Collector
        object.h            # Object System (Values, Strings, Functions etc.)
        alloc.h             # Memory Allocation Utilities
        native.h            # Native Functions
        opcode.h            # Bytecode Opcodes
        unicode.h           # Unicode and Grapheme related utilities
```

### Compilation Pipeline
1. Lexer (`src/lexer.c`): Token source code.
2. Parser (`src/parser.c`): Build Abstract Syntax Tree from tokens.
3. Compiler (`src/compiler.c`): Generate Bytecode from AST
4. Virtual Machine (`src/vm.c`): Stack-based virtual machine executing bytecode.


## Build System

### CMake (Primary)
```shell
# Configure Debug Build
cmake -B build -DCMAKE_BUILD_TYPE=Debug

# Configure Release Build
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build Pankti Binary
cmake --build build --target pankti
```

### Build Options
| Option | Default | Description |
|--------|---------|-------------|
| `USE_NAN_BOXING` | `ON` | `Enable NaN-boxed values` |
| `BUILD_GFX` | `ON` | `Build with graphics standard library support` |
| `CMAKE_BUILD_TYPE` | `Debug` | `Build type: Debug, Release, RelWithDebInfo` |

### C Language Standard
- **C11** is required
- GNU extensions enabled for GCC builds

### Naming Conventions

| Type | Convention | Example |
|------|------------|---------|
| Functions | PascalCase | `NewLexer()`, `VmPush()`, `GcCollect()` |
| Structs/Types | PascalCase with `P` prefix | `PVm`, `PObj`, `PEnv`, `Pgc` |
| Macros | UPPER_SNAKE_CASE | `GC_OBJ_THRESHOLD`, `OP_CONST` |
| Local variables | camelCase | `frameCount`, `objCount` |
| Type aliases | all lowercase | `u8`, `u16`, `u32`, `u64` |
| Static functions | camelCase with prefix | `vmError()`, `vmPrintStackTrace()` |
