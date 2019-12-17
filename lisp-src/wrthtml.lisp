;; File: (load "wrthtml.lisp")
;;uncomment if nlet is not loaded yet
; (defmacro nlet(n letargs &rest body)
;    `(labels ((,n ,(mapcar #'car letargs) ,@body))
;       (,n ,@(mapcar #'cadr letargs)) ))


(defun tit(s) (and (> (length s) 0) (char= (aref s 0) #\#)))

(defun tohtml(xs fname)
  (with-open-file (out fname :direction :output
                          :if-does-not-exist :create
			  :if-exists :append) ;; or :supersede
    (nlet next-line ( (s xs) )
       (cond ( (null s) T)
	     ( (tit (car s)) (format out "<h2>~a</h2>~%" (car s))
	       (next-line (cdr s)))
	     ( T (format out "~a<br/>~%" (car s))
		 (next-line (cdr s)) )) )))

