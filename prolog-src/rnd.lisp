(define-prolog-builtin "random" (var sexpr)
     (unify-ao 1 (random (to-lisp-object sexpr)))
     (prolog-backtrack-or-continue))

