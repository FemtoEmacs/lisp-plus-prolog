#lang racket

(provide logic)
(require ffi/unsafe
         ffi/unsafe/define)

(define-ffi-definer define-prolog 
  (ffi-lib "libcxprolog"))

(define-prolog repl (_fun _string -> _int))

(define (logic s) (repl s))

;; To start Prolog:
;;   > (require "prolog.rkt")
;;   > (logic "He comes/from Bahia")

