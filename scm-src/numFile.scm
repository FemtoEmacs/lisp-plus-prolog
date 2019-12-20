;; (load "numFile.scm")
;; The procedure (rdLines <filename>) reads
;; a Lisp file as a list of lines

(define (read-line p)
  (define (eat p c)
    (if (and (not (eof-object? (peek-char p)))
             (char=? (peek-char p) c))
        (read-char p)))
  (let loop ((c (read-char p)) (line '()))
      (cond [ (eof-object? c)
	      (if (null? line) c
		  (list->string (reverse line)))]
            [ (char=? #\newline c)
              (list->string (reverse line))]
            [else (loop (read-char p) (cons c line))] )) )

(define (port->lines p)
   (let next-line ( (i 1) (x (read-line p)) )
     (cond [ (eof-object? x) #t]
        [else (display "#| ")
           (display (number->string i))
           (cond [ (< i 10)  (display "    |# ")]
                 [ (< i 100) (display "   |# ")]
                 [else  (display "  |# ")])
           (display x) (newline)
           (next-line (+ i 1) (read-line p))] ))) 

(define (rdLines filename)
   (call-with-input-file filename port->lines))

