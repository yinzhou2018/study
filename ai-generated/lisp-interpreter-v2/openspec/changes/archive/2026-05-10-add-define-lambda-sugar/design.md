## Context

The evaluator currently handles `(define name expr)` where `name` is a Symbol and `expr` is any expression. This forces users to write `(define square (lambda (x) (* x x)))` — two levels of nesting just to name a function. All mainstream Lisp dialects (Scheme, Common Lisp, Racket) support the shorthand `(define (name params...) body)`.

The change is isolated to `EvalDefine` in `evaluator.cc` — a single function that currently assumes the first argument is always a Symbol.

## Goals / Non-Goals

**Goals:**
- Accept `(define (name params...) body)` as equivalent to `(define name (lambda (params...) body))`
- Keep existing `(define name expr)` behavior unchanged
- Handle edge cases gracefully (e.g., empty param list, multi-expression body via implicit begin)

**Non-Goals:**
- Variadic parameters (rest args with `.`) — not yet supported by the lambda infrastructure
- Curried define forms like `(define ((foo x) y) ...)` — out of scope
- Recursive define sugar for letrec-style bindings

## Decisions

**Decision 1: Desugar in EvalDefine rather than in the parser or AST**

The sugar is evaluated at runtime in `EvalDefine` rather than in the parser. Rationale:
- The parser is grammar-driven (flex/bison) and adding special-case logic there would complicate the grammar
- Desugaring in the evaluator keeps the change minimal and contained
- The rewritten form `(define name (lambda ...))` reuses existing code paths (`EvalLambda` and `EvalDefine`), reducing duplication

Alternative considered: Transform in `ApplySpecialForm` before dispatching. Rejected because it adds an indirection layer for a single special form.

**Decision 2: Handle via destructuring check, not a separate special form**

When `EvalDefine` receives args whose `car` is a Pair (not a Symbol), it destructures the function name and parameters from that pair. No new special form or syntax is introduced.

Alternative considered: Add a new special form `define-func`. Rejected — diverges from standard Lisp syntax and adds complexity for no benefit.

## Risks / Trade-offs

- **[Risk]**: Ambiguous error messages if the user passes a malformed define-function form (e.g., `(define (1 2) body)`) → **Mitigation**: Validate that the function name position holds a Symbol; throw a clear error otherwise
- **[Trade-off]**: Desugaring at eval time means each call to `define` re-checks the argument shape → negligible in practice since `define` is typically called once per binding at load time
