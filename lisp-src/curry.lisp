(defmacro curry(fn x)
   (let ( (g (gensym)))
      `(lambda(,g) (,fn ,g ,x)) ))

(defun quick-sort(s)
  (cond ((null s) s)
        ((null (cdr s)) s)
        (T (append 
              (quick-sort (remove-if-not (curry string< (car s)) 
                             (cdr s)))
              (list (car s))
              (quick-sort (remove-if-not (curry string>= (car s))
                             (cdr s))) ) )))
