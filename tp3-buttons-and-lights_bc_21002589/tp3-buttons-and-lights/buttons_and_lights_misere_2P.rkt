#lang datalog/sexp
(require racket/list)
;;; predicat next/10
;L'action 'a' inverse la première lampe (inverser = allumer ou éteindre)
;L'action 'b' copie la première lampe sur la 2eme lampe et éteind la première lampe 
;L'action 'c' copie la 2eme lampe sur la 3eme lampe et éteind la 2eme lampe

;; Si le premier élément est égale a 1 inverser la lampe 
(! (:- (next A B C T L A1 B1 C1 T1 L1)
       (> T 0 :- TT) (= TT #t)
       (> A 0 :- AT) (= AT #t) 
       (= 0 A1) (= B B1) (= C C1)
       (- T 1 :- T1) (cons a L :- L1)
       )
   )
(! (:- (next A B C T L A1 B1 C1 T1 L1)
       (> T 0 :- TT) (= TT #t)
       (> A 0 :- AT) (= AT #f)
       (> B 0 :- BT) (= BT #t)
       (= 0 A1) (= A B1) (= C C1)
       (- T 1 :- T1) (cons b L :- L1)
       )
   )
(! (:- (next A B C T L A1 B1 C1 T1 L1)
       (> T 0 :- TT) (= TT #t)
       (> C 0 :- CT) (= CT #t)
       (= A A1) (= B C1)
       (+ B 1 :- BT)(modulo BT 2 :- B1)
       (- T 1 :- T1) (cons c L :- L1)
       )
   )

(! (:- (double_next A B C T L A1 B1 C1 T1 L1)
       (!= T 0) 
       (next A B C T L Y U I O P)
       (next Y U I T P A1 B1 C1 T1 L1)
   )
)

;;; 2 Joueurs
;;; predicat end/4
(! (end 0 0 0 #t))
(! (:- (end A B C R)
       (> A 0 :- A1)
       (> B 0 :- B1) 
       (> C 0 :- C1) 
       (= R #f)
       )
   )

;;; predicat eval/5
;;; 2 joueurs
(! (:- (eval A B C T L)
       (end A B C X)
       (= X #t)
       (= T T)
       (= L '())))
(! (:- (eval A B C T L)
    (!= T 0) 
    (double_next A B C T '() A1 B1 C1 T1 L1)
    (eval A1 B1 C1 T1 L2)
    (cons L1 L2 :- L)))

;;; Zone de Test 3L2P
(? (double_next 1 1 1 1 '() A B C X Y))
(? (next 1 1 1 1 '() A B C X Y))
(? (next 0 1 1 1 '() A B C X Y))
(? (end 0 0 0 X))
(? (end 1 1 1 X))
(? (end 1 1 0 X))
(? (end 0 1 0 X))
(? (eval 1 1 0 2 X))
(? (eval 1 1 1 2 X))
