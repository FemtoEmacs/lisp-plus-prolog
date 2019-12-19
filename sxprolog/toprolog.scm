(load-shared-object "libcxprolog.so")
(define prolog 
  (foreign-procedure "repl" (uptr) integer-32))

(define (prolog-str sz s xptr)
    (let nxt-char [(i 0) ]
      (cond [(>= i sz) 
	     (foreign-set! 'char 
		xptr i (integer->char 0))
	     xptr]
	    [(char=? (string-ref s i) #\/)
	     (foreign-set! 'char xptr i
                      (string-ref s i))
	     (nxt-char (+ i 1))]
	    [else (foreign-set! 'char xptr i
			(string-ref s i))
		  (nxt-char (+ i 1))] )) )

(define (logic s)
  (let [ (xptr (foreign-alloc 500)) ]
      (prolog  (prolog-str (string-length s) s xptr)) ))

     

