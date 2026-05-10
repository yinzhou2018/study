## MODIFIED Requirements

### Requirement: Evaluate conditional expressions
The evaluator SHALL support `(if test then else)` and `(cond (test1 expr1...) ... (else exprN...))`. For `cond`, all expressions after the first in a matching clause SHALL be evaluated in sequence, and the value of the last expression SHALL be returned.

#### Scenario: If true branch
- **WHEN** evaluating `(if #t 1 2)`
- **THEN** the result SHALL be Number(1)

#### Scenario: If false branch
- **WHEN** evaluating `(if #f 1 2)`
- **THEN** the result SHALL be Number(2)

#### Scenario: Cond with matching clause
- **WHEN** evaluating `(cond (#f 1) (#t 2) (else 3))`
- **THEN** the result SHALL be Number(2)

#### Scenario: Cond clause with multiple expressions
- **WHEN** evaluating `(cond (#t (define x 10) (+ x 5)))`
- **THEN** the result SHALL be Number(15)

#### Scenario: Cond else clause with multiple expressions
- **WHEN** evaluating `(cond (#f 1) (else (define x 3) (+ x 2)))`
- **THEN** the result SHALL be Number(5)

### Requirement: Evaluate let expressions
The evaluator SHALL support `(let ((var1 expr1) ...) body...)` for local bindings. All body expressions SHALL be evaluated in sequence, and the value of the last expression SHALL be returned.

#### Scenario: Let with single binding
- **WHEN** evaluating `(let ((x 10)) (+ x 1))`
- **THEN** the result SHALL be Number(11)

#### Scenario: Let with multiple bindings
- **WHEN** evaluating `(let ((x 10) (y 20)) (+ x y))`
- **THEN** the result SHALL be Number(30)

#### Scenario: Let with multiple body expressions
- **WHEN** evaluating `(let ((x 10)) (define y 20) (+ x y))`
- **THEN** the result SHALL be Number(30)

### Requirement: Evaluate lambda expressions
The evaluator SHALL support `(lambda (params...) body...)` to create closures that capture the defining environment. All body expressions SHALL be evaluated in sequence when the lambda is called, and the value of the last expression SHALL be returned.

#### Scenario: Create and call a lambda
- **WHEN** evaluating `((lambda (x) (+ x 1)) 5)`
- **THEN** the result SHALL be Number(6)

#### Scenario: Lambda closure captures environment
- **WHEN** evaluating `(define adder (lambda (n) (lambda (x) (+ n x))))` then `((adder 3) 5)`
- **THEN** the result SHALL be Number(8)

#### Scenario: Lambda with multiple body expressions
- **WHEN** evaluating `((lambda (x) (define y (+ x 1)) (* y 2)) 5)`
- **THEN** the result SHALL be Number(12)
