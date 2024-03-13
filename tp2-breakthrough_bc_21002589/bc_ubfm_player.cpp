#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <iostream>
#include <string>
#include "mybt.h"
#include <unordered_map>
#include <time.h>
#include <vector>

#define IDS_MAX_DEPTH 10
#define TIME_LIMIT 0.95
bt_t ROOT;
bt_t B;
int boardwidth = 0;
int boardheight = 0;
int _max_depth = 10;
bool _solved = false;
bool white_turn = true;
bool debug = false;

bt_move_t _solution[IDS_MAX_DEPTH];
std::unordered_map<std::string, int> H;
bt_t _best_s;
double _chrono = clock();
bool white_turn = true;
bool debug = false;

#define WIN 300

#ifndef VERBOSE_PLAYER
#define VERBOSE_PLAYER
bool verbose = true;
bool showboard_at_each_move = false;
#endif

void name();
void newgame();
void showboard();
void genmove();
void play(char a, char b, char c, char d);

// mes fonctions
double eval(bt_t s);
bt_t selection(bt_t sp);
void expension(bt_t sp);
bt_t backpropagate(bt_t sp);
void ubfm(bt_t s);

void help() {
	fprintf(stderr, "  quit\n");
	fprintf(stderr, "  echo ON | OFF\n");
	fprintf(stderr, "  help\n");
	fprintf(stderr, "  name <PLAYER_NAME>\n");
	fprintf(stderr, "  newgame <NBCOL> <NBLINE>\n");
	fprintf(stderr, "  genmove\n");
	fprintf(stderr, "  play <L0C0L1C1>\n");
	fprintf(stderr, "  showboard\n");
}

void name() {
	printf("= bc_heuristique_player\n\n");
}

void newgame() {
	if((boardheight < 1 || boardheight > 10) && (boardwidth < 1 || boardwidth > 10)) {
		fprintf(stderr, "boardsize is %d %d ???\n", boardheight, boardwidth);
		printf("= \n\n");
		return;
	}
	B.init(boardheight, boardwidth);
	white_turn = true;
	if(verbose) fprintf(stderr, "ready to play on %dx%d board\n", boardheight, boardwidth);
	printf("= \n\n");
}

void showboard() {
	B.print_board(stderr);
	printf("= \n\n");
}

void genmove() {
	int ret = B.endgame();
	if(ret != EMPTY) {
		fprintf(stderr, "game finished\n");
		if(ret == WHITE) fprintf(stderr, "white player wins\n");
		else fprintf(stderr, "black player wins\n");
		printf("= \n\n");
		return;
	}
	bt_move_t m;
	B.play(m);
	if(verbose) {
		m.print(stderr, white_turn, B.nbl);
		fprintf(stderr, "\n");
	}
	white_turn = !white_turn;
	printf("= %s\n\n", m.tostr(B.nbl).c_str());
	if(debug) showboard();
}


// Sert pour jouer nous meme 
void play(char a, char b, char c, char d) {
	bt_move_t m;
	m.line_i = boardheight-(a-'0');
	m.col_i = b-'a';
	m.line_f = boardheight-(c-'0');
	m.col_f = d-'a';
	if(B.can_play(m)) {
		B.play(m);
		if(verbose) {
			m.print(stderr, white_turn, B.nbl);
			fprintf(stderr, "\n");
		}
		white_turn = !white_turn;
	} else {
		fprintf(stderr, "CANT play %d %d %d %d ?\n", m.line_i, m.col_i, m.line_f, m.col_f);
	}
	if(showboard_at_each_move) showboard();
	printf("= \n\n");
}


// Mes fonctions 

std::string get_state(bt_t s) {
	std::string p;
	for (int x = 0; x < boardheight; x++) {
		for (int y = 0; y < boardwidth; y++) {
			p.push_back((char)s.board[x][y]+'0');
		}
	}
	return p;
}

bt_t get_parent(bt_t sp) {
	bt_t s = sp;
	return s;
}

double eval(bt_t s) {
  int i, j;
  double evaluation = 0.0;
  double res = 0.0;
  int turn = s.turn%2;

  for (i = 0; i < s.nbl; i++){
    for (j = 0; j < s.nbc; j++)
    { 
      if (s.board[i][j] == EMPTY) continue;
      else if (turn == WHITE) {
        if (s.board[i][j] == WHITE) {
          if (s.board[i][j] == s.board[0][j]) {
            evaluation += WIN;
            if (debug) printf("win white\t");                
          }
          // nbr d'advairsaires et nbr d'alliés
          evaluation += (10 * s.nbl+1);
          // couverture mutuelle et mise en danger d'un pion ?  
          if(s.board[i+1][j+1] == (s.turn+1)%2) evaluation -= 1;
          if(s.board[i+1][j-1] == (s.turn+1)%2) evaluation -= 1;
          if(s.board[i-1][j+1] == (s.turn+1)%2) evaluation -= 1;
          if(s.board[i-1][j-1] == (s.turn+1)%2) evaluation -= 1;
          ///////////////////////////////////////////////////////////
          if(s.board[i+1][j+1] == s.turn) evaluation += 1;
          if(s.board[i+1][j-1] == s.turn) evaluation += 1;
          if(s.board[i-1][j+1] == s.turn) evaluation += 1;
          if(s.board[i-1][j-1] == s.turn) evaluation += 1;
        }
      }
      else if (turn == (s.turn+1)%2) {
        if (s.board[i][j] == (s.turn+1)%2) {
          if (s.board[i][j] == s.board[s.nbl][j]) {
            evaluation += WIN;
            if (debug) printf("win black\t");                
          }
          // nbr d'advairsaires et nbr d'alliés
          evaluation -= (10 * s.nbl+1);
          // couverture mutuelle et mise en danger d'un pion ?  
          if(s.board[i+1][j+1] == (s.turn+1)%2) evaluation += 1;
          if(s.board[i+1][j-1] == (s.turn+1)%2) evaluation += 1;
          if(s.board[i-1][j+1] == (s.turn+1)%2) evaluation += 1;
          if(s.board[i-1][j-1] == (s.turn+1)%2) evaluation += 1;
          ///////////////////////////////////////////////////////////
          if(s.board[i+1][j+1] == s.turn) evaluation -= 1;
          if(s.board[i+1][j-1] == s.turn) evaluation -= 1;
          if(s.board[i-1][j+1] == s.turn) evaluation -= 1;
          if(s.board[i-1][j-1] == s.turn) evaluation -= 1;
        }
      }
      if (debug) fprintf(stderr, "cases [%d][%d] : %0.1f\t", i,j, evaluation);
      res += evaluation;
    } 
    if (debug) printf("\n");
  }
  if(debug) if(turn%2==0) fprintf(stderr, "White plateau || Eval = %0.1f\n", evaluation);
  if(debug) if(turn%2==1) fprintf(stderr, "Black plateau || Eval = %0.1f\n", evaluation);

  return evaluation;
}


bt_t selection(bt_t sp) {
	/*
	vector<bt_move_t> m = nextMoves(sp);
	{min, max, best} = {infini, -infini, null};
	for (int i = 0; i < sp.nb_moves; i++){
		bt_t spp play(sp, m[i]);
		if H.find(spp) return spp;
		if sp.turn%2 == 0 
			if max < H[spp] {max, best} = {H[spp], spp};
		else 
			if min < H[spp] {min, best} = {H[spp], spp};
	}
	return selection(best)
	*/
	int i;
	bt_t spp;
	bt_t best;
	std::string hash;
	double min = std::numeric_limits<double>::infinity();
	double max = -std::numeric_limits<double>::infinity();
	sp.update_moves();
	for (i = 0; i < sp.nb_moves; i++) {
		spp = sp;
		spp.play(sp.moves[i]);
		hash = get_state(spp);
		if (H.find(hash) != H.end()) return spp;
		if (sp.turn%2 == 0) 
			if (H[hash] > max) {
				max = H[hash];
				best = spp;
			} 
		else { 
			if (H[hash] > min) {
				min  = H[hash];
				best =  spp;
			} 
		}
	}
	return selection(best);
}

void expension(bt_t sp) {
	int i;
	bt_t spp;
	std::string hash;
	sp.update_moves();
	for (i; i < sp.nb_moves; i++) {
		spp = sp;
		spp.play(sp.moves[i]);
		hash = get_state(spp);
		if (H.find(hash) != H.end()) {
			H[hash] = eval(spp);
		}
	}
}

bt_t backpropagate(bt_t sp) {
	int i;
	bt_t spp;
	bt_t p;
	std::string hash, h2;
	h2 = get_state(sp);
	double min = std::numeric_limits<double>::infinity();
	double max = -std::numeric_limits<double>::infinity();
	sp.update_moves();
	if (sp.turn%2 == 0)
	{
		for (i = 0; i < sp.nb_moves; i++)
		{
			spp = sp;
			spp.play(sp.moves[i]);
			hash = get_state(spp);
			if (max < H[hash]) {
				max = H[hash];
			}
		}
		H[h2] = max;
	}
	else {
		for (i = 0; i < sp.nb_moves; i++)
		{
			spp = sp;
			spp.play(sp.moves[i]);
			hash = get_state(spp);
			if (min > H[hash]) {
				min = H[hash];
			}
		}
		H[h2] = min;
	}
	if (h2 == get_state(ROOT)) {
		return;
	}
	p = get_parent(sp);
	return backpropagate(p);
}

void ubfm(bt_t s){
	/*
	reinitialisation de la table de Hashage
	H[s] ajout 0; 
	while not-interrupeted do 
		bt_t s_prime = selection de (s) 
		expension(s_prime)
		backpropagate(s_prime)
	*/ 
	H.clear();
	H.insert({get_state(s), 0});
	_chrono = clock();
	ROOT = s;
	while (((double)(clock() - _chrono)/CLOCKS_PER_SEC) < TIME_LIMIT) {
		bt_t s_prime = selection(s);
		expension(s_prime);
		backpropagate(s_prime);
	}
}

int main(int _ac, char** _av) {
	bool echo_on = false;
	setbuf(stdout, 0);
	setbuf(stderr, 0);
	if(verbose) fprintf(stderr, "bc_heuristique_player started\n");
	char a,b,c,d; // for play cmd
	for (std::string line; std::getline(std::cin, line);) {
		if(verbose) fprintf(stderr, "bc_heuristique_player receive %s\n", line.c_str());
		if(echo_on) if(verbose) fprintf(stderr, "%s\n", line.c_str());
		if(line.compare("quit") == 0) { printf("= \n\n"); break; }
		else if(line.compare("echo ON") == 0) echo_on = true;
		else if(line.compare("echo OFF") == 0) echo_on = false;
		else if(line.compare("help") == 0) help();
		else if(line.compare("name") == 0) name();
		else if(sscanf(line.c_str(), "newgame %d %d\n", &boardheight, &boardwidth) == 2) newgame();
		else if(line.compare("genmove") == 0) genmove();
		else if(sscanf(line.c_str(), "play %c%c%c%c\n", &a,&b,&c,&d) == 4) play(a,b,c,d);
		else if(line.compare("showboard") == 0) showboard();
		else if(line.compare(0,2,"//") == 0) ; // just comments
		else fprintf(stderr, "???\n");
		if(echo_on) printf(">");
	}
	if(verbose) fprintf(stderr, "bye.\n");

	return 0;
}
