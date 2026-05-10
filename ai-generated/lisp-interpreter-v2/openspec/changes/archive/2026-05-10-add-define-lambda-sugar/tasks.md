## 1. Core Implementation

- [x] 1.1 Modify `EvalDefine` in `evaluator.cc` to detect function-form define: when `car` of args is a Pair, extract function name from that pair's `car`, params from that pair's `cdr`, and body from args' `cdr`; construct a lambda internally and bind it with the plain `(define name (lambda ...))` path
- [x] 1.2 Add a private helper `EvalDefineFunction` in `evaluator.cc` (anonymous namespace or private method) if the expanded `EvalDefine` would exceed 40 lines

## 2. Tests

- [x] 2.1 Add test cases to `test.lisp` for the shorthand define syntax: simple two-param function, zero-param function, and closure-returning function
- [x] 2.2 Build the project and run the test file to verify correctness
