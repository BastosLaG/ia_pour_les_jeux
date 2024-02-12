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
int _solution_sized;
bt_t _best_s;
bool white_turn = true;
bool debug = false;

std::map<bt_t, int> H;


#define WIN 300

#ifndef VERBOSE_PLAYER
#define VERBOSE_PLAYER
bool verbose = true;
bool showboard_at_each_move = false;
#endif

void dls_init();
void dls(bt_t s, int d);
void ids(bt_move_t _m);
double eval_heuristique(bt_move_t _m);

std::vector<bt_move_t> nextMoves(bt_t s);
bt_t applyMove(bt_t s, bt_move_t move);
bt_move_t get_heuristique_move();


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


double eval_heuristique(bt_move_t _m) {
  double evaluation = 0.0;
  int turn = B.turn%2;
  
  if(debug) _m.print(stdout, B.turn, B.nbl);

  // Win
  if (turn%2 == WHITE) if(_m.line_f == 0) evaluation += WIN;
  if (turn%2 == BLACK) if(_m.line_f == B.nbl-1) evaluation += WIN;
  // Attack
  if(B.board[_m.line_f][_m.col_f] == (turn+1)%2) evaluation += 100;
  // Case vide 
  if(B.board[_m.line_f][_m.col_f] == EMPTY) evaluation += 50;
  // Safety
  if(B.board[_m.line_f+1][_m.col_f+1] == turn%2) evaluation += 10;
  if(B.board[_m.line_f+1][_m.col_f-1] == turn%2) evaluation += 10;
  if(B.board[_m.line_f-1][_m.col_f+1] == turn%2) evaluation += 10;
  if(B.board[_m.line_f-1][_m.col_f-1] == turn%2) evaluation += 10;
  ///////////////////////////////////////////////////////////////
  if(B.board[_m.line_f+1][_m.col_f+1] == (turn+1)%2) evaluation -= 10;
  if(B.board[_m.line_f+1][_m.col_f-1] == (turn+1)%2) evaluation -= 10;
  if(B.board[_m.line_f-1][_m.col_f+1] == (turn+1)%2) evaluation -= 10;
  if(B.board[_m.line_f-1][_m.col_f-1] == (turn+1)%2) evaluation -= 10; 

  if(debug) if(turn%2==0) fprintf(stderr, "|| White : %c%c -> %c%c || Eval = %0.1f\n", boardheight-(_m.line_i-'0'), 'a'+_m.col_i, boardheight-(_m.line_f-'0'), 'a'+_m.col_f, evaluation);
  if(debug) if(turn%2==1) fprintf(stderr, "|| Black : %c%c -> %c%c || Eval = %0.1f\n", boardheight-(_m.line_i-'0'), 'a'+_m.col_i, boardheight-(_m.line_f-'0'), 'a'+_m.col_f, evaluation);
  
  return evaluation;
}


void dls_init() {
  _max_depth = 10;
  H.clear();
  _solution_sized = 0;
  _best_s = B;
  dls(B, 0);
}


// Implement nextMoves function
std::vector<bt_move_t> nextMoves(bt_t s) {
    std::vector<bt_move_t> moves;
    s.update_moves();
    for (int i = 0; i < s.nb_moves; i++)
    {
      moves.push_back(s.moves[i]);
    }
    return moves;
}

// Implement applyMove function
bt_t applyMove(bt_t s, bt_move_t move) {
    // Apply move to state s and return the new state
    bt_t s_prime = s;
    s_prime.play(move);
    return s_prime;
}

void dls(bt_t s, int d) {
  long unsigned int i;
  double best_s;
  std::vector<bt_move_t> M;

  // if _solution_sized != 0 then return; 
  if (_solution_sized != 0) return;
  // H[s] <- d
  H.insert(std::make_pair(s, d));
  // if s == h(best_s) > h(s) then 
  // best_s <- s;
  
  // if s == WIN then 
  // // solution_size <- d;
  // // return;
  if (best_s == WIN) {
    _solution_sized = d;
    return;
  }
  // if s == LOST or d == _max_depth then return;
  if (best_s <= 0 || d >= _max_depth) return;

  //M <- nextMoves(s);
  M = nextMoves(s);
  for (i = 0; i < M.size(); i++) {
    //s' = applyMove(s, M[i]);
    bt_t s_prime = applyMove(s, M[i]);
    /*s' !€ H or H[s'] > d then
      *solution[d] <- m;    
      *dls(s', d+1);
    */
    dls(s_prime, d + 1);


    // if solution_size != 0 then break;
    if (_solution_sized != 0) break;
  }
}





void ids(bt_move_t _m) {

}



bt_move_t get_heuristique_move(){
  int i;
  int r = 0;
  int r_temp = 0;
  // int _max_depth = 10;
  double h_max = 0;
  
  for (i = 0; i < B.nb_moves; i++){
    r_temp = eval_heuristique(B.moves[i]);
    if (h_max < r_temp) {
      r = i; 
      h_max = r_temp;
    }
  }
  bt_move_t best_move = B.moves[r];



  if(debug) {
    fprintf(stderr, "moves [\n");
    for (i = 0 ; i < B.nb_moves; i++){
      fprintf(stderr, "\t%c%c -> %c%c, \n", boardheight-(B.moves[i].line_i-'0'), 'a'+B.moves[i].col_i , boardheight-(B.moves[i].line_f-'0'), 'a'+B.moves[i].col_f);
    }
    fprintf(stderr, "];\n");  
  }

  if(debug) fprintf(stderr, "Best move [%d] = %0.1f\n", r ,h_max);
  return best_move;
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
