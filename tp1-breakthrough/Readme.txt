////////////////////////
/// TP recherche IDS ///
////////////////////////

q1- Définir :
+ une fonction heuristique pour la variante classique 
+ une recherche IDS en temps limité (présentée au cours1) dans un fichier ids_player.cpp

q2a- Ajouter la variante misère (dans mybt.h et dans run_many_games.pike)
q2b- Ajouter la variante à captures forcées (dans mybt.h et dans run_many_games.pike)
q2c- Ajouter la variante à coups simultanés (dans mybt.h et dans run_many_games.pike)
q2d- Ajouter la variante à information imparfaite (dans mybt.h et dans run_many_games.pike)
q2e- Proposer une solution permettant de réaliser une compétition entre plusieurs programmes


(question q1 obligatoire)
(questions q2a à q3 fixées semi-aléatoirement)

barème :
q1 = 2 pts
q2a = 2 pts
q2b = 2 pts
q2c = 3 pts
q2d = 4 pts
q2e = 4 pts

Présenter votre solution dans 3 semaines.

////////////////////////

Fichiers fournis pour ce TP :
* mybt.h définit les structures bt_piece_t, bt_move_t et bt_t
* bt_piece_t qui modélise une piece
* bt_move_t qui modélise un coup
* bt_t qui modélise le plateau et ses contraintes
* rand_player.cpp est un joueur aléaoire qui supporte le breakthrough text protocol btp
* le protocol btp permet de controler un programme pour jouer a breakthrough
* game1.txt est un exemple de fichier de commandes btp
* run_many_games.pike est un programme pike permettant de faire jouer ensemble des programmes supportant le btp
* Makefile permet de compiler le rand_player
* mk_stats.sh permet de lancer plusieurs parties, stocker les logs et les stats

Pour le moment, on posera pour contrainte de répondre le coup à jouer en 0.1 sec
Au delà de 0.1 sec, l'absence de réponse sera considérée comme un abandon

/////////////////////////
