#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <iostream>
#include <string>
#include "mybt.h"

bt_t B;
int boardwidth = 0;
int boardheight = 0;
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
  printf("= heuristique_player\n\n");
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

double eval(bt_move_t _m) {
  // À implémenter selon vos critères d'évaluation
  double evaluation = 0.0;

  if (B.turn == WHITE) {
    // Win
    if(_m.line_f == B.nbl) evaluation += 100;  

    // Attack
    if(B.board[_m.line_f][_m.col_f] == BLACK) evaluation += 50;
    // Case vide 
    if(B.board[_m.line_f][_m.col_f] == EMPTY) evaluation += 30;

    // Safety
    if(B.board[_m.line_f-1][_m.col_f+1] == WHITE) evaluation += 20;
    if(B.board[_m.line_f-1][_m.col_f-1] == WHITE) evaluation += 20;

    if(B.board[_m.line_f+1][_m.col_f+1] == BLACK) evaluation -= 10;
    if(B.board[_m.line_f+1][_m.col_f-1] == BLACK) evaluation -= 10;

    std::cout << "White [ "<< _m.line_i << ", " << _m.col_i << " ] -> [ " << _m.line_f << ", " << _m.col_f << " ] \nFind : " << B.board[_m.line_f][_m.col_f] << " : " << evaluation << std::endl;
  }
  else if (B.turn == BLACK) {    
    // Win
    if(_m.line_f == 1) evaluation += 100;
    
    // Attack
    if(B.board[_m.line_f][_m.col_f] == WHITE) evaluation += 50;
    // Case Vide
    if(B.board[_m.line_f][_m.col_f] == EMPTY) evaluation += 30;

    // safety
    if(B.board[_m.line_f+1][_m.col_f+1] == BLACK) evaluation += 20;
    if(B.board[_m.line_f+1][_m.col_f-1] == BLACK) evaluation += 20;

    // 
    if(B.board[_m.line_f-1][_m.col_f+1] == WHITE) evaluation -= 10;
    if(B.board[_m.line_f-1][_m.col_f-1] == WHITE) evaluation -= 10;
    
    std::cout << "Black[ "<< _m.line_i << ", " << _m.col_i << " ] -> [ " << _m.line_f << ", " << _m.col_f << " ] \nFind : " << B.board[_m.line_f][_m.col_f] << " : " << evaluation << std::endl;
  }
  
  return evaluation;
}

bt_move_t get_heuristique_move(){
  B.update_moves();
  int r = 0;
  double h_max = -1000.0;
  for (int i = 0 ; i <= B.nb_moves; i++){

    std::cout << B.turn << " eval " << i << " : " << std::endl; 
    std::cout << eval(B.moves[i]) << std::endl;

      if (h_max <= eval(B.moves[i])) {
        r = i; 
        h_max = eval(B.moves[i]);
      }
  }
  std::cout << "H_MAX : " << h_max << " R = " << r << std::endl;
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
}

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
  if(verbose) fprintf(stderr, "heuristique_player started\n");
  char a,b,c,d; // for play cmd
  for (std::string line; std::getline(std::cin, line);) {
    if(verbose) fprintf(stderr, "heuristique_player receive %s\n", line.c_str());
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
