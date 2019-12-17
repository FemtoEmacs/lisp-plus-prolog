(defun fib(n)
   (cond ( (< n 2) (+ n 1))
         ( (= n 2) 3)
         (T  (+ (fib (- n 3))
	        (* (fib (- n 2)) 2)) )))

