; test.lisp - comprehensive test suite

; Arithmetic
(define a (+ 1 2 3 4))
(define b (* 2 3 4))
(define c (- 10 3))
(define d (/ 20 4))

; Variables
(define x 42)

; Lambda and closures
(define square (lambda (x) (* x x)))
(define adder (lambda (n) (lambda (x) (+ n x))))

; Conditionals
(define abs-val (lambda (x) (if (< x 0) (- 0 x) x)))

; List operations
(define lst (list 1 2 3))
(define first (car lst))
(define rest (cdr lst))
(define new-lst (cons 0 lst))

; Let
(define result (let ((a 10) (b 20)) (+ a b)))

; Boolean
(define is-zero (lambda (x) (= x 0)))

; Shorthand define (define (name params...) body...)
(define (add x y) (+ x y))
(define (answer) 42)
(define (make-adder n) (lambda (x) (+ n x)))

; Output
a
b
c
d
x
(square 5)
((adder 3) 5)
(abs-val (- 0 7))
first
rest
new-lst
result
(is-zero 0)
(is-zero 1)
(null? '())
(pair? '(1 2))
(not #f)

; Shorthand define tests
(add 3 4)
(answer)
((make-adder 5) 3)
