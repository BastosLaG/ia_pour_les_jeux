///////////////////
/// TP3 datalog ///
///////////////////

Le jeu buttons and lights est un jeu avec des lampes et des boutons.
Les boutons agissent sur les lampes.
Dans la version classique, l'objectif est d'allumer toutes les lampes.
On commencera par le jeu à 3 lampes. On évoluera ensuite vers le jeu à N lampes. On pourrait jouer à 1 ou plusieurs joueurs, mais on se limitera à la version à 1 joueur.
On représentera les lampes par des valeurs 0 et 1.
0 est une lampe éteinte.
1 est une lampe allumée.
Pour le jeu à 3 lampes, 0 0 0 représente 3 lampes éteintes.

Le premier bouton allume ou éteint la première lampe.
Les boutons suivants copie la lampe précédente dans la lampe suivante et éteint la lampe précédente.

Partant de 0 0 0, appuyer sur le premier bouton donne 1 0 0
Partant de 1 0 0, appuyer sur le premier bouton donne 0 0 0

Partant de 1 0 0, appuyer sur le deuxième bouton donne 0 1 0
Partant de 0 1 0, appuyer sur le deuxième bouton donne 0 0 0

Partant de 0 1 0, appuyer sur le troisième bouton donne 0 0 1
Partant de 0 0 1, appuyer sur le troisième bouton donne 0 0 0

Partant d'un problème, il s'agit de trouver la séquence qui allume toutes les lampes.
On pourra noter les lampes 1 2 3.
Ainsi partant de 0 0 0, la solution la plus courte est 1 2 3 1 2 1

L'objectif de ce TP est d'écrire le programme qui résoud le jeu "Buttons and Lights".
A partir d'une position, il s'agit de donner la séquence de coups pour arriver au but.
Dans la version classique, il faut allumer les lampes.
Dans la version misère, il faut éteindre les lampes.
Dans la version à coups simultanés, on peut jouer plusieurs coups en 1 tour. Le mécanisme des coups de Buttons and Lights fait qu'il ne peut pas y avoir d'interférences entre les coups. Jouer plusieurs fois le même coup a le même effet que le jouer une fois. La version à N coups simultanés est également une version coopérative à N joueurs à information complète (chaque joueur sait ce que les autres joueurs veulent jouer et tous les arrangements sont possibles entre les joueurs).

On utilisera au maximum les prédicats Datalog.
On spécifiera le problème à résoudre en paramètre du programme.
Le programme répondra par la liste des coups à jouer. Dans la version à 1 joueur, on aura une simple liste de coups. Dans la version à 2 joueurs, on aura une liste de couples de coups. Dans la version à N joueurs, on aura une liste de listes de coups.

Possibilité pour le jeu à trois lampes en version classique et à 1 joueur :
1/ Définir un prédicat next/10
 les variables sont les suivantes:
 AVANT l'action :
 *** A : valeur de la première lampe 
 *** B : valeur de la 2eme lampe 
 *** C : valeur de la 3eme lampe 
 *** T : nombre de tours restants 
 *** L : liste des actions déjà appliquées (ou anciennes actions)
 APRES l'action
 *** A1 : valeur de la première lampe 
 *** B1 : valeur de la 2eme lampe 
 *** C1 : valeur de la 3eme lampe 
 *** T1 : nombre de tours restants 
 *** L1 : liste des actions appliquées (i.e. liste des anciennes actions avec en + une nouvelle action) 

L'action 'a' inverse la première lampe (inverser = allumer ou éteindre)
L'action 'b' copie la première lampe sur la 2eme lampe et éteind la première lampe 
L'action 'c' copie la 2eme lampe sur la 3eme lampe et éteind la 2eme lampe

Quand on pose la question :
 (? (next 0 1 1 1 '() A B C X Y))
Le programme répond :
 next(0, 1, 1, 1, '(), 1, 1, 1, 0, '(a)).
 next(0, 1, 1, 1, '(), 0, 0, 1, 0, '(b)).
 next(0, 1, 1, 1, '(), 0, 0, 1, 0, '(c)).

2/ Définir un prédicat end/4
 les variables sont les suivantes
 *** A : valeur de la première lampe 
 *** B : valeur de la 2eme lampe 
 *** C : valeur de la 3eme lampe 
 *** R : résultat de l'évaluation des lampes
     si toutes les lampes sont allumées, R est vrai
     
Quand on pose les questions :
 ? (end 1 1 1 X))
 ? (end 0 1 0 X))
Le programme répond :
 end(1, 1, 1, #t).
 end(0, 1, 0, #f).
 
3/ Définir un prédicat eval/5
 les variables sont les suivantes:
 *** A : valeur de la première lampe 
 *** B : valeur de la 2eme lampe 
 *** C : valeur de la 3eme lampe 
 *** T : nombre de tours restants 
 *** L1 : liste des actions à appliquer pour résoudre le problème posé A B C en moins de T actions

 evaluer/5, c'est regarder si end/4 est vrai
 si end/4 est faux, c'est appeler next/10 pour obtenir A1 B1 C1 et c'est evaluer/5 A1 B1 C1
 a chq fois qu'on apliquer next/10, il faut décrémenter le nombre de tours
 avoir une solution, c'est jouer tant que le nombre de tours est positif
 
Quand on pose la question :
 (? (eval 0 0 0 7 X))
Le programme répond : 
 eval(0, 0, 0, 7, '(a b a c b a)).
 eval(0, 0, 0, 7, '(a b c a b a)).

On mettre des questions exemples commentées en fin de programme
Par exemple, en mode Datalog parenthésé préfixé :
;;; (? (eval 0 0 0 7 X))

(questions du TP3)
Pour chaque question, on commencera par un programme pour la version à 3 lampes puis on continuera par un programme pour la version à N lampes

Après avoir écrit une version utilisant un maximum Datalog, on pourra proposer des améliorations en passant certaines parties en Racket en fonctionnel pour accélérer les calculs. La détection des cycles et des transpositions devraient à priori rester en Datalog. 

Pour information, le programme Datalog parenthésé préfixé décrit ci dessus fait 22 lignes.
On répondra :
* en Datalog parenthésé préfixé pour la version à 3 lampes
* en Datalog parenthésé préfixé dans un programme Racket pour la version à N lampes

On pourra ajouter la fonction suivante d'initialisation aléatoire des lampes pour la version à N lampes.
(define (init-lights nb-lights) 
  (build-list nb-lights (lambda (x) (random 2)))
)

q1a- résoudre la version classique à 1 joueur (4 points)
q1b- résoudre la version misère à 1 joueur (4 points)
q1c- résoudre la version classique à 2 joueurs (6 points)
q1d- résoudre la version misère à 2 joueurs (6 points)


