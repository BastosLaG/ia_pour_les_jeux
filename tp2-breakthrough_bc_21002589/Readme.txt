//////////////////////////
/// TP2 recherche MCTS ///
//////////////////////////

(questions du TP1)
q1- Définir :
+ une fonction heuristique pour la variante classique à coups alternés et information complète 
+ réaliser un joueur heuristique dans un heuristic_player.cpp
+ une recherche IDS en temps limité (présentée au cours1) dans un ids_player.cpp

q2a- Réaliser les joueurs heuristique et IDS pour la variante misère à coups alternés et information complète
q2b- ??? Vous en êtes ou ???
q2c- Réaliser les joueurs heuristique et IDS pour la variante classique à coups simultanés et information complète
q2d- Réaliser les joueurs heuristique et IDS pour la variante classique à coups alternés et dark
q2e- Solution permettant de réaliser une compétition entre plusieurs programmes

barème :
q1 = 2 pts
q2a = 2 pts
q2b = 2 pts
q2c = 3 pts
q2d = 4 pts
q2e = 4 pts

(questions du TP2)
q3- Réaliser une recherche MCTS
q4- Réaliser une recherche UBFMS avec la fonction heuristique de q1

q3 = 6 pts
q4 = 6 pts

////////////////////////

Evolution des fichiers fournis pour ce TP :
* mybt.h définit les structures bt_piece_t, bt_move_t et bt_t
* bt_piece_t qui modélise une piece
* bt_move_t qui modélise un coup
* bt_t qui modélise le plateau et ses contraintes
* rand_player.cpp est un joueur aléaoire qui supporte le breakthrough text protocol btp
* genmove donne le meilleur coup mais ne le joue pas
* le protocol btp permet de controler un programme pour jouer a breakthrough
* le protocol varie selon les modes de jeux (alt, sim, fui, dak, bld)
* game1_*.txt permettent de tester les joueurs avec des commandes btp
* run_many_games.pike permet de faire jouer ensemble des joueurs avec des commandes btp
* run_many_simultaneous_games.pike variante à coups simultanés
* run_many_ii_games.pike variante à information incomplète
* Makefile permet de compiler le rand_player
* mk_stats.sh permet de lancer plusieurs parties, stocker les logs et les stats

Pour le moment, on posera pour contrainte de répondre le coup à jouer en 0.1 sec
Au delà de 0.1 sec, l'absence de réponse sera considérée comme un abandon

///////////////////////// protocol btp breakthrough classique à coups alternés et information complète

protocole de communication d'une partie à breakthrough (vu de l'arbitre)
[arbitre ---> j0 ] name
[j0 ---> arbitre ] = NOM_J0
[l'arbitre note le nom de j0]
[arbitre ---> j1 ] name
[j1 ---> arbitre ] = NOM_J1
[l'arbitre note le nom de j1]
[arbitre ---> j0 ] new_game 6 4
[j0 ---> arbitre ] =
[arbitre ---> j1 ] new_game 6 4
[j1 ---> arbitre ] =
[arbitre ---> j0 ] genmove
[j0 ---> arbitre ] = COUP0
[l'arbitre joue COUP0 sur sa board]
[déclare j1 vainqueur en cas de coup illégal de j0]
[arbitre ---> j0 ] play COUP0
[j0 ---> arbitre ] = 
[arbitre ---> j1 ] play COUP0
[j1 ---> arbitre ] = 
[arbitre ---> j1 ] genmove
[j1 ---> arbitre ] = COUP1
[l'arbitre joue COUP1 sur sa board]
[déclare j0 vainqueur en cas de coup illégal de j1]
[arbitre ---> j0 ] play COUP1
[j0 ---> arbitre ] = 
[arbitre ---> j1 ] play COUP1
[j1 ---> arbitre ] = 
...
[pour afficher la board avec un joueur à information complète]
[arbitre ---> j0 ] showboard
[j0 affiche la board sur stderr]
[j0 ---> arbitre ] = 
...
[pour afficher la board dans les log avec un joueur à information complète]
[arbitre ---> j0 ] strboard
[j0 retourne la board à l'arbitre]
[j0 ---> arbitre ] = BOARD
[reste à la faire afficher par l'arbitre]
...
[quand l'arbitre constate une fin de partie ou un temps dépassé]
[arbitre ---> j0 ] quit
[j0 ---> arbitre ] = 
[arbitre ---> j1 ] quit
[j1 ---> arbitre ] =

/////////////////////////////////// classique

On compte les victoires et les pions sur le plateau en fin de partie
Les pions donnent des points en cas de victoire.
Les pions retirent des points en cas de défaite.
Les points permettent de départager les ex-aequo sur le nombre de victoires.

/////////////////////////////////// variante misère

A la fin de partie, il faut inverser le gagnant de la version classique.
Les points sont également inversés.

/////////////////////////////////// variante à coups forcés
(à faire)

/////////////////////////////////// variante à coups simultanés

A chaque tour, chaque joueur annonce son coup
Si les coups sont en conflits, aucun coup n'est joué
Après 3 tours consécutifs en conflit, la partie s'arrête par un match nul
Si les deux joueurs gagnent au même tour, ils ont 0.4 victoires et points de plus chacun.

/////////////////////////////////// variante à information incomplète 

Chaque joueur ne voit pas tout
Variante dark : Chaque joueur voit les pièces adverses qui sont à une case devant ces pièces
Variante blind : Chaque joueur ne voit que ses pièces

Variante blind à finir : 
Quand il y a capture, que faire ? indiquer aux joueurs ?
Quand un coup se solde par un échec, que faire ? 
