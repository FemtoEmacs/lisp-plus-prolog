(define-syntax (while-do stx)
  (syntax-case stx ()
    [ (kwd condition the-return . bdy)
       (datum->syntax #'kwd
         (let [ (c (syntax->datum #'condition))
	        (r (syntax->datum #'the-return))
		(body (syntax->datum #'bdy))]
	   `(do () ((not ,c) ,r) ,@body))) ]))

(define-syntax (curry stx)
   (syntax-case stx ()
      [ (kwd fun arg)
        (datum->syntax #'kwd
           (let [ (fn (syntax->datum #'fun))
	          (x  (syntax->datum #'arg))
	          (g  (gensym "var"))]
	      `(lambda(,g) (,fn ,g ,x)) ))] ))

