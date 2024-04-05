#lang datalog/sexp

(! (is_even 0 true))
(! (is_even 1 false))
(! (:- (is_even X Y)
      (!= X 0) (!= X 1) (- X 2 :- X1) (is_even X1 Y)
   ))
(? (is_even 10 Y))
(? (is_even 11 Y))


(! (is_even 0 true))
(! (is_even 1 false))
(! (:- (is_even X Y)
(> X 1 :- C) (= C #t) (- X 2 :- X1) (is_even X1 Y)
))
(? (is_even 10 Y))
(? (is_even 11 Y))

(! (:- (is_even X Y)
(modulo X 2 :- X1) (= X1 0) (= Y true)
))
(! (:- (is_even X Y)
(modulo X 2 :- X1) (= X1 1) (= Y false)
))
(? (is_even 10 Y))
(? (is_even 11 Y))

(! (integer_sum 1 1))
(! (:- (integer_sum X Y)
       (!= X 1)
       (- X 1 :- X1)
       (integer_sum X1 R1)
       (+ X R1 :- Y) 
    ))

(? (integer_sum 10 X))
(? (integer_sum 100 X))

(! (fibo 0 0))
(! (fibo 1 1))
(! (:- (fibo N F)
       (!= N 0) (!= N 1) (- N 1 :- N1) (- N 2 :- N2)
       (fibo N1 F1) (fibo N2 F2) (+ F1 F2 :- F)
   )
)

(? (fibo 30 F))
(? (fibo 5 F))

(! (f1 (cons 1 2)))
(! (:- (f2 X Y Z) (cons X Y :- Z)))
(! (f3 0 (list 0)))
(! (:- (f3 X L)
       (> X 0 :- A) (= A #t) (- X 1 :- X1) (F3 X1 L1) (cons X L1 :- L)))

(? (f1 X))
(? (f2 1 2 X))
(? (f2 3 (list 4) X))
(? (f2 3 '(4) X))
(? (f3 10 X))



;;; on déclare le fait suivant
(! (:- (all-1 -1 -1 #t)))
;;; V est faux si la première valeur est différente de -1,
;;; et quelquesoit la deuxième valeur
(! (:- (all-1 A B #f) (!= A -1) (= B B)))
;;; V est faux si la deuxième valeur est différente de -1,
;;; et quelquesoit la première valeur
(! (:- (all-1 A B #f) (= A A) (!= B -1)))
(? (all-1 -1 -1 X))
(? (all-1 2 -1 X))
(? (all-1 -1 23 X))
(? (all-1 17 32 X))

