;; Some functions defined in McCarthy's original lisp paper I wanted test

;; Returns the sign of x (-1,0,1)
(define sgn (lambda x (if (< x 0) -1 (if (eq x 0) 0 1))))

;; factorial of n (note fails if n < 0)
(define factorial (lambda n (if (eq n 0) 1 (* n (factorial (- n 1))) )))

;; gcd (recursive and needs remainder function), not fully defined
(define gcd (lambda (m n) 
	      (if (> m n) 
		  (gcd n m) 
		(if (eq 0 (rem n m)) 
		    m 
		  (gcd (rem n m) m)))))

;; TODO: newton's approximation of sqrt, page 5

;; AND, OR, NOT
;; XXX: He also defines  p->q which has the truth table for NOT p OR q
;; Ill ignore it.
(define and (lambda (p q) (if p q 0)))
(define or (lambda (p q) (if p 1 q)))
(define NOT (lambda p (if p 0 1)))

;; TODO: ff on page 12
;; TODO: subst on page 13
;; TODO: equal on page 13
;; TODO: null on page 14
;; TODO: append on page 14
;; TODO: among on page 14
;; TODO: pair on page 14
;; TODO: assoc on page 15
;; TODO: sublis on page 15
