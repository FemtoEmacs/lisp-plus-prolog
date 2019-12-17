(defparameter *x* 42)

(setf  (symbol-function 'xor)
  (lambda (A B)
     (or (and A (not B))
	 (and (not A) B)) ))

(defparameter xor-structure
  '(or (and A
            (not B))
       (and (not A) 
            B)))
