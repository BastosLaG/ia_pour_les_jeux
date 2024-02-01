CC=g++
CFLAGS=-std=c++11 -Wall -O2

all: rand_player ids_player

##### BREAKTHROUGH
rand_player: mybt.h rand_player.cpp
	$(CC) $(CFLAGS) rand_player.cpp -o $@

ids_player: mybt.h ids_player.cpp
	$(CC) $(CFLAGS) ids_player.cpp -o $@

clean:
	rm -f rand_player ids_player
