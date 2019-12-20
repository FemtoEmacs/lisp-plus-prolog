(define (avg xs)
  (let nxt [(s xs) (acc 0) (n 0)] 
    (cond [ (and (null? s) (= n 0)) 0]
      [ (null? s) (/ acc n) ]
      [else (nxt (cdr s) (+ (car s) acc)
		 (+ n 1.0))] )))
