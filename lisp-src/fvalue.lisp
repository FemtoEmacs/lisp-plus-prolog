;; File: (load "fvalue.lisp")

(defun  future-value(pv i n)
   (* (expt (+ (/ i 100.0) 1) n) 
      pv)
);;end defun
