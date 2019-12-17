;; File: celsius.lisp
; Comments are prefixed with semicolon
;; Some people use two semicolons to
;; make comments more visible.

(defun c2f(x)
(- (/ (* (+ x 40) 9) 5.0) 40)
);;end defun

(defun f2c(x)
(- (/ (* (+ x 40) 5.0) 9) 40)
);;end defun

