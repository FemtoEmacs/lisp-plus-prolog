;; File: (load "pmt.scm")

(define (pmt p ;; present value
	     i ;; interest
	     n );; number of periods
  (define (pmt-aux r rn)
     (/ (* r p rn)
        (- rn 1)))
  (pmt-aux (/ i 100.0)
	   (expt (+ 1.0 (/ i 100)) n))
);;end of define

