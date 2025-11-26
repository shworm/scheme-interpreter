(letrec ((a 3) (b (lambda () (+ a 1))) (c (lambda () (+ (b) 1))))
  (c))
