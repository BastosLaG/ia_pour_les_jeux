CC=g++
CFLAGS=-std=c++11 -Wall -O2

all: bc_rand_player bc_ubfm_player

##### BREAKTHROUGH
bc_rand_player: mybt.h rand_player.cpp
	$(CC) $(CFLAGS) rand_player.cpp -o $@

bc_ubfm_player: mybt.h bc_ubfm_player.cpp
	$(CC) $(CFLAGS) bc_ubfm_player.cpp -o $@

clean:
	rm -f bc_rand_player bc_ubfm_player