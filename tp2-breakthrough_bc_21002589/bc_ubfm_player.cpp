#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <iostream>
#include <string>
#include "mybt.h"
#include <unordered_map>
#include <time.h>
#include <vector>
#include <limits>

#define IDS_MAX_DEPTH 10
#define TIME_LIMIT 0.95
#define WIN 300

bt_t B;
int boardwidth = 0;
int boardheight = 0;
bool white_turn = true;

// mes vars
bt_t _best_s;
std::string ROOT;
bt_move_t _solution[IDS_MAX_DEPTH];
std::unordered_map<std::string, int> H;
double _chrono = clock();
bool debug = false;

// mes fonctions
std::string get_state(bt_t s);
bt_t get_parent(bt_t sp);
double eval(bt_t s);
bt_t selection(bt_t sp);
void expension(bt_t sp);
void backpropagate(bt_t sp);
void ubfm(bt_t s);

// Fonctions cours
void help();
void name();
void setname(char *name);
void newgame();
void mode(std::string _strmode);
void showmodes();
void showboard();
void getstrboard();
void getstrboard(char _turn);
void setboard(int _game_turn, char _str_board[MAX_COLS*MAX_LINES]);
void genmove();
void genmove(char _turn);
void play(char a0, char a1, char a2, char a3);
void play(char a0, char a1, char a2, char a3, char b0, char b1, char b2, char b3);

char playername[128];

#ifndef VERBOSE_PLAYER
#define VERBOSE_PLAYER
bool verbose = true;
bool showboard_at_each_move = false;
#endif

int main(int _ac, char** _av) {
  bool echo_on = false;
  setbuf(stdout, 0);
  setbuf(stderr, 0);
  if(verbose) fprintf(stderr, "player started\n");
  char a0,a1,a2,a3; // for play cmd0
  char b0,b1,b2,b3; // for play cmd0 cmd1
  int game_turn;
  char str_board[MAX_COLS*MAX_LINES]; 
  char newname[128];
  char newmode[128];
  int newseed = 0;
  for (std::string line; std::getline(std::cin, line);) {
    if(verbose) fprintf(stderr, "%s receive %s\n", playername, line.c_str());
    if(echo_on) if(verbose) fprintf(stderr, "%s\n", line.c_str());
    bool cmd_ok = false;
    if(sscanf(line.c_str(), "mode %s\n", newmode) == 1) { cmd_ok=true; mode(newmode);}
    else if(line.compare("showmodes") == 0) { cmd_ok=true; showmodes();}
    else if(line.compare("quit") == 0) { cmd_ok=true; printf("= \n\n"); break;}
    else if(line.compare("echo ON") == 0) { cmd_ok=true; echo_on = true;}
    else if(line.compare("echo OFF") == 0) { cmd_ok=true; echo_on = false;}
    else if(line.compare("help") == 0) { cmd_ok=true; help();}
    else if(line.compare("name") == 0) { cmd_ok=true; name();}
    else if(sscanf(line.c_str(), "setname %s\n", newname) == 1) { cmd_ok=true; setname(newname);}
    else if(sscanf(line.c_str(), "newgame %d %d\n", &boardheight, &boardwidth) == 2) 
      { cmd_ok=true; newgame();}
    else if(line.compare("showboard") == 0) { cmd_ok=true; showboard();}
    else if(line.compare("strboard") == 0) { cmd_ok=true; getstrboard();}
    else if(sscanf(line.c_str(), "seed %d\n", &newseed) == 1) 
      { cmd_ok=true; srand(newseed); printf("= \n\n"); }
    
    if(B.alternate_or_simultaneous == 0) {
      if(line.compare("genmove") == 0) { cmd_ok=true; genmove();}
      else if(sscanf(line.c_str(), "play %c%c%c%c\n", &a0,&a1,&a2,&a3) == 4) 
        { cmd_ok=true; play(a0,a1,a2,a3);}
    }

    if(B.alternate_or_simultaneous == 1) {
      if(sscanf(line.c_str(), "genmove %c\n", &a0) == 1) { cmd_ok=true; genmove(a0);}
      else if(sscanf(line.c_str(), "play %c%c%c%c %c%c%c%c\n", &a0,&a1,&a2,&a3,&b0,&b1,&b2,&b3) == 8) 
        { cmd_ok=true; play(a0,a1,a2,a3,b0,b1,b2,b3);}
    }

    if(B.fullinfo_or_dark_or_blind == 1 || B.fullinfo_or_dark_or_blind == 2) {
      if(sscanf(line.c_str(), "setboard %d %s\n", &game_turn, str_board) == 2) 
        { cmd_ok=true; setboard(game_turn, str_board);}
      else if(sscanf(line.c_str(), "strboard %c\n", &a0) == 1) { cmd_ok=true; getstrboard(a0);}
    }        

    if(line.compare(0,2,"//") == 0) { cmd_ok=true; } // just comments
    if(cmd_ok == false) fprintf(stderr, "??? [%s] \n", line.c_str());
    if(echo_on) printf(">");
  }
  if(verbose) fprintf(stderr, "bye.\n");
  return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Mes fonctions 
//////////////////////////////////////////////////////////////////////////////////////////////////////////
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
		if (sp.turn%2 == 0) {
			if (H[hash] > max) {
				max = H[hash];
				best = spp;
			} 
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
	for (i = 0; i < sp.nb_moves; i++) {
		spp = sp;
		spp.play(sp.moves[i]);
		hash = get_state(spp);
		if (H.find(hash) != H.end()) {
			H[hash] = eval(spp);
		}
	}
}

void backpropagate(bt_t sp) {
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
	if (h2 == ROOT) {
		return;
	}
	p = get_parent(sp);
	return backpropagate(p);
}

void ubfm(bt_t s){
	H.clear();
	H.insert({get_state(s), 0});
	_chrono = clock();
	ROOT = get_state(s);
	while (((double)(clock() - _chrono)/CLOCKS_PER_SEC) < TIME_LIMIT) {
		bt_t s_prime = selection(s);
		expension(s_prime);
		backpropagate(s_prime);
	}
	fprintf(stderr, "Time to finished : %d", ((double)(clock() - _chrono)/CLOCKS_PER_SEC) < TIME_LIMIT);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// fonction cours
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void help() {
  fprintf(stderr, "  mode cla | mis (classic OR misere)\n"); 
  fprintf(stderr, "  mode alt | sim (alternate OR simultaneous)\n");
  fprintf(stderr, "  mode fui | drk | bld (fullinfo OR dark OR blind)\n");
  fprintf(stderr, "  showmodes\n");  
  fprintf(stderr, "  quit\n");
  fprintf(stderr, "  echo ON | OFF\n");
  fprintf(stderr, "  help\n");
  fprintf(stderr, "  name\n");
  fprintf(stderr, "  setname <NEW_PLAYER_NAME>\n");
  fprintf(stderr, "  newgame <NBCOL> <NBLINE>\n");
  fprintf(stderr, "  showboard (print board on stderr)\n");
  fprintf(stderr, "  strboard\n");
  fprintf(stderr, "  seed SEED\n");
  fprintf(stderr, "-- alt MODE\n");
  fprintf(stderr, "  genmove\n");
  fprintf(stderr, "  play <L0C0L1C1>\n");
  fprintf(stderr, "-- sim MODE\n");
  fprintf(stderr, "  genmove <TURN>\n");
  fprintf(stderr, "  play <L0C0L1C1> <L0C0L1C1>\n");
  fprintf(stderr, "-- dark and blind MODES\n");
  fprintf(stderr, "  setboard <GAME_TURN> <BOARD_SEEN>\n"); // player's side cmd
  fprintf(stderr, "  strboard <TURN>\n"); // referee's side cmd on oracle
}

void name() {
  printf("= %s\n\n", playername);
}

void setname(char* _name) {
  strncpy(playername, _name, 128);
  printf("= \n\n");
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

void mode(std::string _strmode) {
  if(_strmode.compare("cla") == 0) B.set_cla_or_mis(0);
  if(_strmode.compare("mis") == 0) B.set_cla_or_mis(1);
  if(_strmode.compare("alt") == 0) B.set_alt_or_sim(0);
  if(_strmode.compare("sim") == 0) B.set_alt_or_sim(1);
  if(_strmode.compare("fui") == 0) B.set_fullinfo_or_dark_or_blind(0);
  if(_strmode.compare("drk") == 0) B.set_fullinfo_or_dark_or_blind(1);
  if(_strmode.compare("bld") == 0) B.set_fullinfo_or_dark_or_blind(2);
  if(verbose) {
    if(_strmode.compare("cla") == 0) fprintf(stderr, " set CLASSIC\n");
    if(_strmode.compare("mis") == 0) fprintf(stderr, " set MISERE\n");
    if(_strmode.compare("alt") == 0) fprintf(stderr, " set ALTERNATE\n");
    if(_strmode.compare("sim") == 0) fprintf(stderr, " set SIMULTANEOUS\n");
    if(_strmode.compare("fui") == 0) fprintf(stderr, " set FULLINFO\n");
    if(_strmode.compare("drk") == 0) fprintf(stderr, " set DARK\n");
    if(_strmode.compare("bld") == 0) fprintf(stderr, " set BLIND\n");
  }
  printf("= \n\n");
}

void showmodes() {
  if(B.classic_or_misere == 0) fprintf(stderr, " classic\n");
  if(B.classic_or_misere == 1) fprintf(stderr, " misere\n");
  if(B.alternate_or_simultaneous == 0) fprintf(stderr, " alternate\n");
  if(B.alternate_or_simultaneous == 1) fprintf(stderr, " simultaneous\n");
  if(B.fullinfo_or_dark_or_blind == 0) fprintf(stderr, " fullinfo\n");
  if(B.fullinfo_or_dark_or_blind == 1) fprintf(stderr, " dark\n");
  if(B.fullinfo_or_dark_or_blind == 2) fprintf(stderr, " blind\n");
  printf("= \n\n");
}

void showboard() {
  B.print_board(stderr);
  printf("= \n\n");
}

void getstrboard() {
  char strb[128];
  B.get_board(strb);
  printf("= %s\n\n", strb);  
}

void getstrboard(char _turn) {
  if(_turn == '@') {
    char ret[128];
    B.get_board(BLACK, ret);    
    printf("= %s\n\n", ret);
  } else {
    char ret[128];
    B.get_board(WHITE, ret);    
    printf("= %s\n\n", ret);
  }
}
// to define the board with variants DARK and BLIND
// at each turn, before asking genmove, it is needed to set the board
// board 0 ??????...oooooo
// board 1 @@@@@@.o.??????
// board 2 ???.@@@o..ooooo
void setboard(int _game_turn, char _str_board[MAX_COLS*MAX_LINES]) {  
  white_turn = ((_game_turn%2)==0);
  B.init_board(_game_turn, _str_board);
  printf("= \n\n");
}
void play(char a0, char a1, char a2, char a3) {
  bt_move_t m;
  m.line_i = boardheight-(a0-'0');
  m.col_i = a1-'a';
  m.line_f = boardheight-(a2-'0');
  m.col_f = a3-'a';
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
void play(char a0, char a1, char a2, char a3, char b0, char b1, char b2, char b3) {
  bt_move_t m0;
  m0.line_i = boardheight-(a0-'0');
  m0.col_i = a1-'a';
  m0.line_f = boardheight-(a2-'0');
  m0.col_f = a3-'a';
  bt_move_t m1;
  m1.line_i = boardheight-(b0-'0');
  m1.col_i = b1-'a';
  m1.line_f = boardheight-(b2-'0');
  m1.col_f = b3-'a';
  if(B.can_simultaneous_play(m0,m1)) {
    B.play(m0,m1);
    //fprintf(stderr, "after play : %d white / %d black\n", B.nb_white_pieces, B.nb_black_pieces);
  } else {
    fprintf(stderr, "CANT play simultaneously %d %d %d %d with %d %d %d %d\n", m0.line_i, m0.col_i, m0.line_f, m0.col_f, m1.line_i, m1.col_i, m1.line_f, m1.col_f);
  }
  if(showboard_at_each_move) showboard();
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
  bt_move_t m = B.get_rand_move();
  printf("= %s\n\n", m.tostr(B.nbl).c_str());
}
void genmove(char _turn) {  
  if(_turn == '@') {
    bt_move_t m = B.get_rand_move(BLACK);
    printf("= %s\n\n", m.tostr(B.nbl).c_str());
  } else {
    bt_move_t m = B.get_rand_move(WHITE);
    printf("= %s\n\n", m.tostr(B.nbl).c_str());
  }
}
