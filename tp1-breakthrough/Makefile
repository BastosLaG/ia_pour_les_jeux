CC=g++
CFLAGS=-std=c++11 -Wall -O2

all: rand_player ids_player

##### BREAKTHROUGH
rand_player: mybt.h rand_player.cpp
	$(CC) $(CFLAGS) rand_player.cpp -o $@

ids_player: mybt.h bc_ids_player.cpp
	$(CC) $(CFLAGS) bc_ids_player.cpp -o $@

clean:
	rm -f rand_player ids_player
