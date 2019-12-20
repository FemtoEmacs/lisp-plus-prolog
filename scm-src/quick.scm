(define (quick-sort s)
  (cond [(null? s) s]
     [(null? (cdr s)) s]
     [else 
      (append 
         (quick-sort (filter (curry string<? (car s)) 
			     (cdr s)))
         (list (car s))
         (quick-sort (filter (curry string>=? (car s))
			     (cdr s))) )]))
