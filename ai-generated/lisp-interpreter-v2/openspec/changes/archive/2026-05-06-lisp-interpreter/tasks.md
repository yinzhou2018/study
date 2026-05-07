## 1. Project Setup & Build System

- [x] 1.1 Create project directory structure (src/, include/, openspec/)
- [x] 1.2 Create CMakeLists.txt with C++20 standard, flex/bison find_package, readline detection
- [x] 1.3 Verify clean build produces empty executable scaffolding

## 2. Value Type & AST

- [x] 2.1 Define Value variant type in value.h: Nil, Bool, Number, String, Symbol, Pair, Lambda
- [x] 2.2 Implement Pair struct with car/cdr as shared_ptr<Value>
- [x] 2.3 Implement Lambda struct capturing params, body, and closure environment
- [x] 2.4 Implement Value display/pretty-print (to_string) in value.cc

## 3. Environment

- [x] 3.1 Define Environment class in environment.h with unordered_map + parent pointer
- [x] 3.2 Implement lookup (recursive chain search) in environment.cc
- [x] 3.3 Implement define (bind in current scope) and set (update existing binding)

## 4. Lexer (flex)

- [x] 4.1 Create lisp.l flex definition with token rules: LPAREN, RPAREN, QUOTE, NUMBER, STRING, SYMBOL, BOOLEAN
- [x] 4.2 Implement comment skipping (semicolon line comments)
- [x] 4.3 Implement location tracking (line/column via yylloc)
- [x] 4.4 Verify lexer tokenizes sample inputs correctly

## 5. Parser (bison)

- [x] 5.1 Create lisp.y bison grammar with C++ parser class (%language "c++")
- [x] 5.2 Define grammar rules: expr → atom | list | quote_expr; list → LPAREN expr* RPAREN
- [x] 5.3 Implement AST construction in semantic actions (build Pair chains)
- [x] 5.4 Implement quote expansion ('expr → (quote expr))
- [x] 5.5 Implement error reporting with location info (custom yy::parser::error)
- [x] 5.6 Verify parser produces correct AST for sample S-expressions

## 6. Evaluator

- [x] 6.1 Create evaluator.h/evaluator.cc with eval() entry point dispatching on Value type
- [x] 6.2 Implement self-evaluating types: Number, String, Boolean evaluate to themselves
- [x] 6.3 Implement symbol lookup in environment chain
- [x] 6.4 Implement special forms: define, if, cond, let, quote, lambda
- [x] 6.5 Implement function application (lambda + built-in)
- [x] 6.6 Implement built-in arithmetic: +, -, *, /
- [x] 6.7 Implement built-in list ops: car, cdr, cons, list, null?, pair?
- [x] 6.8 Implement built-in comparison: =, <, >, <=, >=
- [x] 6.9 Implement built-in boolean: and, or, not
- [x] 6.10 Implement runtime error reporting (unbound variable, arity, type mismatch)

## 7. REPL Mode

- [x] 7.1 Create repl.h/repl.cc with read-eval-print loop
- [x] 7.2 Implement multi-line input detection (unmatched parentheses → continuation prompt)
- [x] 7.3 Integrate GNU readline for line editing and history (conditional compilation)
- [x] 7.4 Implement graceful exit on Ctrl+D / (exit) / (quit)
- [x] 7.5 Verify REPL handles interactive sessions correctly

## 8. File Execution Mode

- [x] 8.1 Create main.cc with CLI argument parsing (no args → REPL, one arg → file mode)
- [x] 8.2 Implement file reader: open file, parse all expressions, evaluate sequentially
- [x] 8.3 Implement file error handling (file not found, syntax/runtime errors with line info)
- [x] 8.4 Verify file execution with test .lisp scripts

## 9. Integration & Testing

- [x] 9.1 Create test.lisp with sample programs covering all features
- [x] 9.2 End-to-end test: REPL session with define, lambda, closures, conditionals
- [x] 9.3 End-to-end test: file execution with multi-expression programs
- [x] 9.4 Verify all coding standard compliance (.h/.cc, member count ≤7, function ≤40 lines)
