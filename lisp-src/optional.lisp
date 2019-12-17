;; Example of optional parameters

(defun avg(s &optional (sum 0) (n 0))
  (cond ( (and (null s) (= n 0)) 0)
        ( (null s) (/ sum n) )
        (T (avg (cdr s) (+ (car s) sum) (+ n 1.0)) )))
