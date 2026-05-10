## Why

The current `define` only supports `(define name expr)`, making function definitions verbose: users must write `(define square (lambda (x) (* x x)))`. Most Lisp dialects provide syntactic sugar `(define (name params...) body)` that expands to the lambda form automatically. Adding this removes boilerplate and improves readability.

## What Changes

- Extend `EvalDefine` to detect when the first argument is a list (Pair) instead of a Symbol
- When a list is detected, destructure it into function name and parameters, then internally rewrite to `(define name (lambda (params...) body))`
- All existing `define` behavior remains unchanged — this is purely additive

## Capabilities

### New Capabilities
<!-- This is a small extension of define; no separate capability needed — it falls under the existing evaluator capability -->

### Modified Capabilities
- `evaluator`: The define requirement is extended to support `(define (name params...) body)` as syntactic sugar for `(define name (lambda (params...) body))`

## Impact

- **evaluator.cc**: `EvalDefine()` gets a fast-path check: if `car` of args is a Pair instead of a Symbol, desugar into lambda form
- **evaluator.h**: Helper `EvalDefineFunction` may be added if the logic warrants a separate function (stays under 40 lines)
- **test.lisp**: New test cases for the shorthand syntax
