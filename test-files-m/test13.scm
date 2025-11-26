(letrec ((is-even? (lambda (n) 
                        (if (equal? n 0)
                            #t
                            (is-odd? (+ n -1)))))

         (is-odd? (lambda (n)
                    (if (equal? n 0)
                        #f
                        (is-even? (+ n -1))))))
    
    (is-odd? 11))