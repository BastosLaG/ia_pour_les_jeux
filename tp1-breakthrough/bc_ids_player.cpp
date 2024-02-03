#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <iostream>
#include <string>
#include "mybt.h"

bt_t B;
int boardwidth = 0;
int boardheight = 0;
int _max_depth = 0;
bool white_turn = true;


#ifndef VERBOSE_RAND_PLAYER
#define VERBOSE_RAND_PLAYER
bool verbose = true;
bool showboard_at_each_move = false;
#endif

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

double eval_heuristique(bt_move_t _m, int nbi) {
  double evaluation = 0.0;
  // int i;
  _m.print(stdout, B.turn, B.nbl);
  int turn = B.turn%2;
  // printf(" turn = %d ", B.turn);
  // for (i = 0; i <= nbi; i++){
    if (turn == WHITE) {
      // Win
      if(_m.line_f == B.nbl) evaluation += 300 /* *(nbi - i) */;  
      // Attack
      if(B.board[_m.line_f][_m.col_f] == BLACK) evaluation += 100;
      // Case vide 
      if(B.board[_m.line_f][_m.col_f] == EMPTY) evaluation += 50;
      // Safety
      if(B.board[_m.line_f+1][_m.col_f+1] == WHITE) evaluation += 10;
      if(B.board[_m.line_f+1][_m.col_f-1] == WHITE) evaluation += 10;
      if(B.board[_m.line_f-1][_m.col_f+1] == WHITE) evaluation += 10;
      if(B.board[_m.line_f-1][_m.col_f-1] == WHITE) evaluation += 10;
      ///////////////////////////////////////////////////////////////
      if(B.board[_m.line_f+1][_m.col_f+1] == BLACK) evaluation -= 10;
      if(B.board[_m.line_f+1][_m.col_f-1] == BLACK) evaluation -= 10;
      if(B.board[_m.line_f-1][_m.col_f+1] == BLACK) evaluation -= 10;
      if(B.board[_m.line_f-1][_m.col_f-1] == BLACK) evaluation -= 10;

      fprintf(stderr, " || White : %c%c -> %c%c || Eval = %0.1f\n", boardheight-(_m.line_i-'0'), 'a'+_m.col_i, boardheight-(_m.line_f-'0'), 'a'+_m.col_f, evaluation);
    }
    else if (turn == BLACK) {
      // Win
      if(_m.line_f == 0) evaluation += 300;
      // Attack
      if(B.board[_m.line_f][_m.col_f] == WHITE) evaluation += 100;
      // Case Vide
      if(B.board[_m.line_f][_m.col_f] == EMPTY) evaluation += 50;
      // safety
      if(B.board[_m.line_f+1][_m.col_f+1] == BLACK) evaluation += 10;
      if(B.board[_m.line_f+1][_m.col_f-1] == BLACK) evaluation += 10;
      if(B.board[_m.line_f-1][_m.col_f+1] == BLACK) evaluation += 10;
      if(B.board[_m.line_f-1][_m.col_f-1] == BLACK) evaluation += 10;
      /////////////////////////////////////////////////////////////// 
      if(B.board[_m.line_f+1][_m.col_f+1] == WHITE) evaluation -= 10;
      if(B.board[_m.line_f+1][_m.col_f-1] == WHITE) evaluation -= 10;
      if(B.board[_m.line_f-1][_m.col_f+1] == WHITE) evaluation -= 10;
      if(B.board[_m.line_f-1][_m.col_f-1] == WHITE) evaluation -= 10;
      
      fprintf(stderr, "|| Black : %c%c -> %c%c || Eval = %0.1f\n", boardheight-(_m.line_i-'0'), 'a'+_m.col_i, boardheight-(_m.line_f-'0'), 'a'+_m.col_f, evaluation);
    }
  // }
  return evaluation;
}

bt_move_t get_heuristique_move(){
  int i;
  int r = 0;
  int r_temp = 0;
  B.update_moves();
  double h_max = eval_heuristique(B.moves[0], 10);

  fprintf(stderr, "moves [\n");
  for (i = 0 ; i < B.nb_moves; i++){
    fprintf(stderr, "\t%c%c -> %c%c, \n", boardheight-(B.moves[i].line_i-'0'), 'a'+B.moves[i].col_i , boardheight-(B.moves[i].line_f-'0'), 'a'+B.moves[i].col_f);
  }
  fprintf(stderr, "];\n");
  
  for (i = 0; i < B.nb_moves; i++){
    r_temp = eval_heuristique(B.moves[i], 10);
      if (h_max < r_temp) {
        r = i; 
        h_max = r_temp;
      }
  }
  fprintf(stderr, "Best move [%d] = %0.1f\n", r ,h_max);
  return B.moves[r];
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
  bt_move_t m = get_heuristique_move();
  B.play(m);
  if(verbose) {
    m.print(stderr, white_turn, B.nbl);
    fprintf(stderr, "\n");
  }
  white_turn = !white_turn;
  printf("= %s\n\n", m.tostr(B.nbl).c_str());
  showboard();
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
