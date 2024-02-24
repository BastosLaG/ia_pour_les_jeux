 #include <cstdio>
#include <cstdlib>
#include <string.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include "mybt.h"

bt_t B;

int boardwidth = 0;
int boardheight = 0;
int _max_depth = 10;

int _solution_size;
bt_t _best_s;
bool white_turn = true;
bool debug = false;

std::map<bt_t, int> H;
std::map<int, bt_move_t> _solution;


#define WIN 300

#ifndef VERBOSE_PLAYER
#define VERBOSE_PLAYER
bool verbose = true;
bool showboard_at_each_move = false;
#endif


void dls(bt_t s, int d);
void ids(bt_t _m);
std::vector<bt_move_t> nextMoves(bt_t s);
bt_t applyMove(bt_t s, bt_move_t move);
bool check_win(bt_t p);
bool check_loose(bt_t p);


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


double eval_heuristique(bt_t p) {
  int i, j;
  double evaluation = 0.0;
  int turn = p.turn%2;

  for (i = 0; i < p.nbl; i++){
    for (j = 0; j < p.nbc; j++)
    {
      if (turn == WHITE) {
        if (p.board[i][j] == p.board[0][j] && p.board[i][j] == WHITE) 
          evaluation += WIN;
        // nbr d'advairsaires et nbr d'alliés
        if(p.board[i][j] == (turn+1)%2) evaluation -= (5 * p.nbl);
        if(p.board[i][j] == turn) evaluation += (5 * abs(i - p.nbl));
      }
      if (turn == BLACK){
        if (p.board[i][j] == p.board[p.nbl-1][j] && p.board[i][j] == BLACK) 
          evaluation += WIN;
        // nbr d'advairsaires et nbr d'alliés
        if(p.board[i][j] == (turn+1)%2) evaluation -= (5 * abs(i - p.nbl));
        if(p.board[i][j] == turn) evaluation += (5 * p.nbl);
      }
      // couverture mutuelle et mise en danger d'un pion ? 
      if(p.board[i+1][j+1] == turn) evaluation += 1;
      if(p.board[i+1][j-1] == turn) evaluation += 1;
      if(p.board[i-1][j+1] == turn) evaluation += 1;
      if(p.board[i-1][j-1] == turn) evaluation += 1;
      ///////////////////////////////////////////////////////////
      if(p.board[i+1][j+1] == (turn+1)%2) evaluation -= 1;
      if(p.board[i+1][j-1] == (turn+1)%2) evaluation -= 1;
      if(p.board[i-1][j+1] == (turn+1)%2) evaluation -= 1;
      if(p.board[i-1][j-1] == (turn+1)%2) evaluation -= 1; 
    } 
  }
  
  return evaluation;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


std::vector<bt_move_t> nextMoves(bt_t s) {
  std::vector<bt_move_t> moves;
  s.update_moves();
  for (int i = 0; i < s.nb_moves; i++)
  {
    moves.push_back(s.moves[i]);
  }
  return moves;
}

bt_t applyMove(bt_t s, bt_move_t move) {
  bt_t s_prime = s;
  s_prime.play(move);
  return s_prime;
}

bool check_win(bt_t p){
  int turn = p.turn%2;
  if (turn == WHITE) {
    for (int i = 0; i < p.nbc; i++) {
      if (p.board[0][i] == WHITE) return true;
    }
    return false;
  }
  else if (turn == BLACK) {
    for (int i = 0; i < p.nbc; i++) {
      if (p.board[p.nbl][i] == BLACK) return true;
    }
    return false;
  }
  return false;
}
bool check_loose(bt_t p){
  int turn = p.turn%2;
  if (turn == WHITE) {
    for (int i = 0; i < p.nbc; i++) {
      if (p.board[p.nbl][i] == BLACK) return true;
    }
    return false;
  }
  else if (turn == BLACK) {
    for (int i = 0; i < p.nbc; i++) {
      if (p.board[0][i] == WHITE) return true;      
    }
    return false;
  }
  return false;
}

void dls(bt_t s, int d) {
  long unsigned int i;
  std::vector<bt_move_t> M;

  if (_solution_size != 0) return;
  H.insert(std::pair<bt_t,int> (s, d));

  if (eval_heuristique(_best_s) > eval_heuristique(s)){
    _best_s = s;
  }
  if (check_win(s)) {
    _solution_size = d;
    return;
  }
  if (check_loose(s) || d >= _max_depth) return;
  M = nextMoves(s);
  for (i = 0; i < M.size(); i++) {
    bt_t s_prime = applyMove(s, M[i]);
    auto recherche = H.find(s_prime);
    if (recherche != H.end() && recherche->second >= d ){
      _solution.insert(std::pair<int,bt_move_t> (d, M[i]));
      dls(s_prime, d+1);
    }
    if (_solution_size != 0) break;
  }
}

void ids(bt_t p) {
  _solution.clear();
  _solution_size = 0;
  _best_s = p;
  for (int d = 1; d < _max_depth; d++) {
    H.clear();
    _max_depth = d;
    dls(p, 0);
    if (_solution_size != 0) break;
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
  ids(B);
  bt_move_t m = _solution[1];
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
