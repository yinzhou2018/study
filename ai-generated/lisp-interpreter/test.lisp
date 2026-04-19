(
  ; 计算斐波纳西数列
  (define (fib n)
    (if (< n 2)
        n
        (+ (fib (- n 1)) (fib (- n 2)))))

  (print (fib 10))

  ; 插入排序
  (define (insert element sorted-list)
    (if (null? sorted-list)
        (list element)
        (if (< element (car sorted-list))
            (cons element sorted-list)
            (cons (car sorted-list) (insert element (cdr sorted-list))))))

  (define (insertion-sort list)
    (if (null? list)
        '()
        (insert (car list) (insertion-sort (cdr list)))))

  ; 测试: 对列表 (5 2 9 1 5 6) 进行排序
  (print (insertion-sort (list 5 2 9 1 5 6)))

  ; 测试: 空列表
  (print (insertion-sort '()))

  ; 测试: 已排序列表
  (print (insertion-sort (list 1 2 3 4 5)))

  ; 牛顿法求平方根
  (define (sqrt-newton x)
    (define (improve guess)
      (/ (+ guess (/ x guess)) 2))
    (define (good-enough? guess)
      (< (abs (- (* guess guess) x)) 0.00001))
    (define (sqrt-iter guess)
      (if (good-enough? guess)
          guess
          (sqrt-iter (improve guess))))
    (sqrt-iter 1.0))

  ; 测试: 求 2、9、0.25 的平方根
  (print (sqrt-newton 2))
  (print (sqrt-newton 9))
  (print (sqrt-newton 0.25))
)