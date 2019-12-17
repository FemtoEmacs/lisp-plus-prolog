(defmacro nlet(n letargs &rest body)
  `(labels ((,n ,(mapcar #'car letargs) ,@body))
      (,n ,@(mapcar #'cadr letargs)) ))

(defun fibo(i)
  (nlet fn+1 ( (n i) (fn 2) (fn-1 1) )
    (if (< n 2) fn (fn+1 (- n 1) (+ fn fn-1) fn)) ))

