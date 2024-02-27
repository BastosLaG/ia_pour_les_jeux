#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <string>
#include <vector>
#include <string.h>
#include <iostream>
#include <map>
#include <unordered_map>
#include <functional>
#include <time.h>
#include "mybt.h"

#define IDS_MAX_DEPTH 10
#define TIME_LIMIT 0.095

bt_t B;

int boardwidth = 0;
int boardheight = 0;
int _max_depth = 10;
int _solution_size;
int _player = EMPTY;
double _chrono = clock();

bt_move_t _solution[IDS_MAX_DEPTH];
std::unordered_map<std::string, int> H;
bt_t _best_s;
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

std::string get_state(bt_t s);
void dls(bt_t s, int d);
void ids();

void genmove();
void play(char a, char b, char c, char d);

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
  printf("= bc_ids_player\n\n");
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


double bt_t::eval() {
  int i, j;
  double evaluation = 0.0;
  double res = 0.0;
  int turn = bt_t::turn%2;

  for (i = 0; i < bt_t::nbl; i++){
    for (j = 0; j < bt_t::nbc; j++)
    { 
      if (bt_t::board[i][j] == EMPTY) continue;
      else if (turn == WHITE) {
        if (bt_t::board[i][j] == WHITE) {
          if (bt_t::board[i][j] == bt_t::board[0][j]) {
            evaluation += WIN;
            if (debug) printf("win white\t");                
          }
          // nbr d'advairsaires et nbr d'alliés
          evaluation += (10 * bt_t::nbl+1);
          // couverture mutuelle et mise en danger d'un pion ?  
          if(bt_t::board[i+1][j+1] == (bt_t::turn+1)%2) evaluation -= 1;
          if(bt_t::board[i+1][j-1] == (bt_t::turn+1)%2) evaluation -= 1;
          if(bt_t::board[i-1][j+1] == (bt_t::turn+1)%2) evaluation -= 1;
          if(bt_t::board[i-1][j-1] == (bt_t::turn+1)%2) evaluation -= 1;
          ///////////////////////////////////////////////////////////
          if(bt_t::board[i+1][j+1] == bt_t::turn) evaluation += 1;
          if(bt_t::board[i+1][j-1] == bt_t::turn) evaluation += 1;
          if(bt_t::board[i-1][j+1] == bt_t::turn) evaluation += 1;
          if(bt_t::board[i-1][j-1] == bt_t::turn) evaluation += 1;
        }
      }
      else if (turn == (bt_t::turn+1)%2) {
        if (bt_t::board[i][j] == (bt_t::turn+1)%2) {
          if (bt_t::board[i][j] == bt_t::board[bt_t::nbl][j]) {
            evaluation += WIN;
            if (debug) printf("win black\t");                
          }
          // nbr d'advairsaires et nbr d'alliés
          evaluation -= (10 * bt_t::nbl+1);
          // couverture mutuelle et mise en danger d'un pion ?  
          if(bt_t::board[i+1][j+1] == (bt_t::turn+1)%2) evaluation += 1;
          if(bt_t::board[i+1][j-1] == (bt_t::turn+1)%2) evaluation += 1;
          if(bt_t::board[i-1][j+1] == (bt_t::turn+1)%2) evaluation += 1;
          if(bt_t::board[i-1][j-1] == (bt_t::turn+1)%2) evaluation += 1;
          ///////////////////////////////////////////////////////////
          if(bt_t::board[i+1][j+1] == bt_t::turn) evaluation -= 1;
          if(bt_t::board[i+1][j-1] == bt_t::turn) evaluation -= 1;
          if(bt_t::board[i-1][j+1] == bt_t::turn) evaluation -= 1;
          if(bt_t::board[i-1][j-1] == bt_t::turn) evaluation -= 1;
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


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string get_state(bt_t s) {
  std::string p;
  for (int x = 0; x < boardheight; x++) {
    for (int y = 0; y < boardwidth; y++) {
      p.push_back((char)s.board[x][y]+'0');
    }
  }
  return p;
}

void dls(bt_t s, int d) {
  if ((_solution_size != 0) || ((double)(clock() - _chrono)/CLOCKS_PER_SEC) > TIME_LIMIT) return;
  H.insert({get_state(s), d});
  if (_best_s.eval() < s.eval()){
    _best_s = s;
  }
  if (s.endgame() == _player) {
    _solution_size = d;
    return;
  }
  if ((s.endgame() != EMPTY && s.endgame() != _player) || d >= _max_depth) return;
  s.update_moves();
  for (int i = 0; i < s.nb_moves; i++) {
    if (debug) fprintf(stderr, "|| %c%c -> %c%c ||\n", boardheight-(s.moves[i].line_i-'0'), 'a'+s.moves[i].col_i, boardheight-(s.moves[i].line_f-'0'), 'a'+s.moves[i].col_f);
    bt_t s_prime = s;
    s_prime.play(s.moves[i]);
    if (debug) s_prime.print_board(stderr);
    
    if (H.find(get_state(s_prime)) == H.end() || H[get_state(s_prime)] > d ) {
      _solution[d] = s.moves[i];
      dls(s_prime, d+1);
      if (debug) fprintf(stderr, "YES BUDDY !!! \n");
    }
    if ((_solution_size != 0) || ((double)(clock() - _chrono)/CLOCKS_PER_SEC) > TIME_LIMIT) break;
  }
}

void ids() {
  _chrono = clock();
  _player = (white_turn) ? WHITE : BLACK;
  _solution_size = 0;
  _best_s = B;
  for (int d = 1; d < IDS_MAX_DEPTH; d++) {
    H.clear();
    _max_depth = d;
    dls(B, 0);
    if ((_solution_size != 0) || ((double)(clock() - _chrono)/CLOCKS_PER_SEC) > TIME_LIMIT) break;
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void genmove() {
  int ret = B.endgame();
  if(ret != EMPTY) {
    fprintf(stderr, "game finished\n");
    if(ret == WHITE) fprintf(stderr, "white player wins\n");
    else fprintf(stderr, "black player wins\n");
    printf("= \n\n");
    return;
  }
  ids();
  B.play(_solution[0]);
  if(verbose) {
    _solution[0].print(stderr, white_turn, B.nbl);
    fprintf(stderr, "\n");
  }
  white_turn = !white_turn;
  printf("= %s\n\n", _solution[0].tostr(B.nbl).c_str());
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

int main(int _ac, char** _av) {
  bool echo_on = false;
  setbuf(stdout, 0);
  setbuf(stderr, 0);
  if(verbose) fprintf(stderr, "bc_ids_player started\n");
  char a,b,c,d; // for play cmd
  for (std::string line; std::getline(std::cin, line);) {
    if(verbose) fprintf(stderr, "bc_ids_player receive %s\n", line.c_str());
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
