CC=g++
CFLAGS=-std=c++11 -Wall -O2

all: bc_rand_player bc_heuristique_player bc_ids_player tournois

##### BREAKTHROUGH
bc_rand_player: mybt.h rand_player.cpp
	$(CC) $(CFLAGS) rand_player.cpp -o $@

bc_heuristique_player: mybt.h bc_ids_player.cpp
	$(CC) $(CFLAGS) bc_heuristique_player.cpp -o $@

bc_ids_player: mybt.h bc_ids_player.cpp
	$(CC) $(CFLAGS) bc_ids_player.cpp -o $@

tournois: mybt.h bc_ids_player.cpp bc_heuristique_player.cpp rand_player.cpp
	$(CC) $(CFLAGS) rand_player.cpp -o Player1
	$(CC) $(CFLAGS) bc_heuristique_player.cpp -o Player2
	$(CC) $(CFLAGS) rand_player.cpp -o Player3
	$(CC) $(CFLAGS) bc_heuristique_player.cpp -o Player4
	$(CC) $(CFLAGS) rand_player.cpp -o Player5
	$(CC) $(CFLAGS) bc_heuristique_player.cpp -o Player6
	$(CC) $(CFLAGS) bc_ids_player.cpp -o Player7
	$(CC) $(CFLAGS) rand_player.cpp -o Player8

clean:
	rm -f bc_rand_player bc_heuristique_player bc_ids_player Player1 Player2 Player3 Player4 Player5 Player6 Player7 Player8 