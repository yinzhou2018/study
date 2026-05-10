## Purpose

Core evaluation engine for the Lisp interpreter, handling arithmetic, special forms, built-in operations, and runtime error reporting.

## Requirements

### Requirement: Evaluate arithmetic expressions
The evaluator SHALL support `+`, `-`, `*`, `/` on numbers with arbitrary argument count (for + and *) or 2 arguments (for - and /).

#### Scenario: Add two numbers
- **WHEN** evaluating `(+ 1 2)`
- **THEN** the result SHALL be Number(3)

#### Scenario: Add multiple numbers
- **WHEN** evaluating `(+ 1 2 3 4)`
- **THEN** the result SHALL be Number(10)

#### Scenario: Divide numbers
- **WHEN** evaluating `(/ 10 3)`
- **THEN** the result SHALL be Number(3) (integer division)

### Requirement: Evaluate variable binding with define
The evaluator SHALL support `(define name expr)` to bind a value in the current environment. It SHALL also support `(define (name params...) body)` as syntactic sugar for `(define name (lambda (params...) body))`.

#### Scenario: Define and use a variable
- **WHEN** evaluating `(define x 42)` followed by `x`
- **THEN** the result of the second expression SHALL be Number(42)

#### Scenario: Define a function using shorthand syntax
- **WHEN** evaluating `(define (add x y) (+ x y))` followed by `(add 3 4)`
- **THEN** the result of the second expression SHALL be Number(7)

#### Scenario: Define a zero-argument function using shorthand syntax
- **WHEN** evaluating `(define (answer) 42)` followed by `(answer)`
- **THEN** the result of the second expression SHALL be Number(42)

#### Scenario: Shorthand function creates a closure
- **WHEN** evaluating `(define (make-adder n) (lambda (x) (+ n x)))` followed by `((make-adder 5) 3)`
- **THEN** the result SHALL be Number(8)

### Requirement: Evaluate lambda expressions
The evaluator SHALL support `(lambda (params...) body)` to create closures that capture the defining environment.

#### Scenario: Create and call a lambda
- **WHEN** evaluating `((lambda (x) (+ x 1)) 5)`
- **THEN** the result SHALL be Number(6)

#### Scenario: Lambda closure captures environment
- **WHEN** evaluating `(define adder (lambda (n) (lambda (x) (+ n x))))` then `((adder 3) 5)`
- **THEN** the result SHALL be Number(8)

### Requirement: Evaluate conditional expressions
The evaluator SHALL support `(if test then else)` and `(cond (test1 expr1) ... (else exprN))`.

#### Scenario: If true branch
- **WHEN** evaluating `(if #t 1 2)`
- **THEN** the result SHALL be Number(1)

#### Scenario: If false branch
- **WHEN** evaluating `(if #f 1 2)`
- **THEN** the result SHALL be Number(2)

#### Scenario: Cond with matching clause
- **WHEN** evaluating `(cond (#f 1) (#t 2) (else 3))`
- **THEN** the result SHALL be Number(2)

### Requirement: Evaluate let expressions
The evaluator SHALL support `(let ((var1 expr1) ...) body)` for local bindings.

#### Scenario: Let with single binding
- **WHEN** evaluating `(let ((x 10)) (+ x 1))`
- **THEN** the result SHALL be Number(11)

#### Scenario: Let with multiple bindings
- **WHEN** evaluating `(let ((x 10) (y 20)) (+ x y))`
- **THEN** the result SHALL be Number(30)

### Requirement: Evaluate quote
The evaluator SHALL support `(quote expr)` to return expr unevaluated.

#### Scenario: Quote a list
- **WHEN** evaluating `'(1 2 3)`
- **THEN** the result SHALL be the list Pair(Number(1), Pair(Number(2), Pair(Number(3), Nil)))

### Requirement: Evaluate list operations
The evaluator SHALL support built-in `car`, `cdr`, `cons`, `list`, `null?`, `pair?`.

#### Scenario: Car of a list
- **WHEN** evaluating `(car '(1 2 3))`
- **THEN** the result SHALL be Number(1)

#### Scenario: Cdr of a list
- **WHEN** evaluating `(cdr '(1 2 3))`
- **THEN** the result SHALL be Pair(Number(2), Pair(Number(3), Nil))

#### Scenario: Cons to build a pair
- **WHEN** evaluating `(cons 1 '(2 3))`
- **THEN** the result SHALL be Pair(Number(1), Pair(Number(2), Pair(Number(3), Nil)))

#### Scenario: List constructor
- **WHEN** evaluating `(list 1 2 3)`
- **THEN** the result SHALL be Pair(Number(1), Pair(Number(2), Pair(Number(3), Nil)))

#### Scenario: Null check
- **WHEN** evaluating `(null? '())`
- **THEN** the result SHALL be Boolean(true)

### Requirement: Evaluate comparison and boolean operations
The evaluator SHALL support `=`, `<`, `>`, `<=`, `>=`, `and`, `or`, `not`.

#### Scenario: Numeric equality
- **WHEN** evaluating `(= 1 1)`
- **THEN** the result SHALL be Boolean(true)

#### Scenario: Less than
- **WHEN** evaluating `(< 1 2)`
- **THEN** the result SHALL be Boolean(true)

#### Scenario: Logical and
- **WHEN** evaluating `(and #t #t)`
- **THEN** the result SHALL be Boolean(true)

### Requirement: Report runtime errors
The evaluator SHALL report meaningful errors for: unbound variables, wrong argument counts, type mismatches.

#### Scenario: Unbound variable
- **WHEN** evaluating `undefined-var`
- **THEN** the evaluator SHALL report an error "unbound variable: undefined-var"

#### Scenario: Wrong argument count
- **WHEN** evaluating `(+ 1)` (wrong arity for binary -)
- **THEN** the evaluator SHALL report an arity error
