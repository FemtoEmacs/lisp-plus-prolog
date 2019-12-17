(defparameter f (cons 2 3))

(defparameter g (cons 4 5))

(defun add(x y)
   (let ( (numerator (+ (* (car x) (cdr y))
                        (* (car y) (cdr x)) ))
          (denominator  (* (cdr x) (cdr y)) ))
      (cons numerator denominator))) 
