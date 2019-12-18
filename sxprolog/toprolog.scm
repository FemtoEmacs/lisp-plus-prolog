(load-shared-object "libcxprolog.so")
(define prolog 
  (foreign-procedure "repl" (integer-32 uptr) integer-32))

(define (prolog-str sz s xptr)
    (let nxt-char [(i 0) (n 0)]
      (cond [(>= i sz) 
	     (foreign-set! 'char 
		xptr i (integer->char 0))
	     (display "n= ") (display n) (newline)
	     (values n xptr)]
	    [(char=? (string-ref s i) #\/)
	     (foreign-set! 'char xptr i
                      (string-ref s i))
	     (nxt-char (+ i 1) (+ n 1))]
	    [else (foreign-set! 'char xptr i
			(string-ref s i))
		  (nxt-char (+ i 1) n)] )) )

(define (logic s)
  (let [ (xptr (foreign-alloc 500)) ]
     (call-with-values
       (lambda() (prolog-str (string-length s) s xptr)) prolog)))
     

