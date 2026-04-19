# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Code Style

代码风格参照 `.clang-format` 配置：
- 基于 Chromium 风格
- 2 空格缩进，不使用 Tab
- 大括号不换行 (Attach)
- 单行函数仅限内联函数
- 列宽限制 120 字符

## Build and Run

```bash
make        # Build the interpreter
make clean  # Clean build artifacts
./lisp      # Run the REPL
```

## Architecture

This is a Lisp interpreter built with Flex (lexer), Bison (parser), and C++17.

### Data Flow
```
Input → Flex (lexer.l) → Tokens → Bison (parser.y) → AST → Evaluator (eval.cpp) → Result
```

### Key Components

- **src/ast.hpp**: Core value types (Number, String, Symbol, List, Lambda, BuiltinFunc) and the Env class for variable bindings. All values are heap-allocated via `shared_ptr<Value>` (aliased as `ValuePtr`).

- **src/lexer.l**: Flex lexer that tokenizes: `NUMBER`, `STRING`, `SYMBOL`, `LPAREN`, `RPAREN`, `QUOTE`. Comments start with `;`.

- **src/parser.y**: Bison grammar producing `ValuePtr` AST nodes. Uses `%code requires` to include `ast.hpp` before the union definition. The parser outputs to global `parsedResult`.

- **src/eval.cpp**: Recursive evaluator with special forms (`if`, `define`, `lambda`, `quote`, `begin`) and builtin functions (`+`, `-`, `*`, `/`, `<`, `>`, `=`, `cons`, `car`, `cdr`, `list`, `print`, `null?`). Creates the global environment via `createGlobalEnv()`.

### Build Artifacts

Flex/Bison outputs go to `build/`:
- `build/lexer.cpp` - Generated lexer
- `build/parser.cpp` / `build/parser.hpp` - Generated parser

### Supported Language Features

- Numbers (int/float), strings, symbols, lists
- Arithmetic: `+`, `-`, `*`, `/`
- Comparison: `<`, `>`, `=`
- List ops: `cons`, `car`, `cdr`, `list`
- Control: `if`, `define`, `lambda`, `begin`, `quote`
- REPL handles multi-line input (tracks paren balance)
- File support: 可选的根括号包裹所有代码

### File Format

文件执行支持两种格式：

1. **无根括号**（传统格式）：
   ```lisp
   (define x 1)
   (define y 2)
   (+ x y)
   ```

2. **有根括号**（包裹格式）：
   ```lisp
   ((define x 1)
    (define y 2)
    (+ x y))
   ```