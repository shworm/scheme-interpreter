(letrec ((sum (lambda (x)
                (if (equal? x 0)
                    0
                    (+ x (sum (+ x -1)))))))
  (sum 5))
