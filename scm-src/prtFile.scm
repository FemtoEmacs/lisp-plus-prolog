;; Racket: uncomment the three lines below
;#lang racket
;(provide rdLines)
;(define get-line read-line)

(define (port->lines p)
   (let next-line ( (i 1) (x (get-line p)) )
     (cond [ (eof-object? x) #t]
        [else (display "#| ")
              (display (number->string i))
              (cond [ (< i 10)  (display "  |# ")]
                    [else  (display " |# ")])
              (display x) (newline)
              (next-line (+ i 1) (get-line p))] ))) 

(define (rdLines filename)
   (call-with-input-file filename port->lines))
