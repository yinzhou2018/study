## MODIFIED Requirements

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
