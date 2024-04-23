#ifndef MYBT_H
#define MYBT_H
#include <cstdio>
#include <cstdlib>
// values on the board
#define WHITE 0
#define BLACK 1
#define EMPTY 2
#define UNSEEN 3
// these values are also used for endgame
// WHITE or BLACK when one player wins
// EMPTY when the game is not finished
// DRAW when both players win with simultaneous moves variant
#define DRAW 4

char* cboard = (char*)"o@.?";

// print black in red (as bg is black... black is printed in red)
// comment the following #define USE_COLOR to print without color
//#define USE_COLOR

struct bt_piece_t {
	int line; int col;
};
struct bt_move_t {
	int line_i; int col_i;
	int line_f; int col_f;

	// all moves are printed without ambiguity
	// white in its color
	// black in red color
	void print(FILE* _fp, bool _white, int _nbl) {
		if(_white) {
			fprintf(_fp, "%d%c%d%c", _nbl-line_i, 'a'+col_i, _nbl-line_f, 'a'+col_f);
		} else {
#ifdef USE_COLOR
			fprintf(_fp, "\x1B[31m%d%c%d%c\x1B[0m", _nbl-line_i, 'a'+col_i, _nbl-line_f, 'a'+col_f);
#else
			fprintf(_fp, "%d%c%d%c", _nbl-line_i, 'a'+col_i, _nbl-line_f, 'a'+col_f);
#endif /* USE_COLOR */
		}
	}
	std::string tostr(int _nbl) {
		char ret[16];
		snprintf(ret, sizeof(ret), "%d%c%d%c", _nbl-line_i, 'a'+col_i, _nbl-line_f, 'a'+col_f);
		return std::string(ret);
	}
};

// alloc default 10x10
// standard game in 8x8
#define MAX_LINES 10
#define MAX_COLS 10

// rules reminder :
// pieces moves from 1 square in diag and in front
// pieces captures only in diag
// i.e. to go forward, square must be empty
struct bt_t {
	int nbl;
	int nbc;
	int board[MAX_LINES][MAX_COLS];
	int turn;
	int classic_or_misere;
	int alternate_or_simultaneous;
	int fullinfo_or_dark_or_blind;

	bt_piece_t white_pieces[2*MAX_LINES];
	int nb_white_pieces;
	bt_piece_t black_pieces[2*MAX_LINES];
	int nb_black_pieces;
	bt_move_t moves[3*2*MAX_LINES];
	int nb_moves;
	// last turn of moves update
	int turn_of_last_moves_update;
	int color_of_last_moves_update;

	void init(int _nbl, int _nbc);
	void set_cla_or_mis(int _v);
	void set_alt_or_sim(int _v);
	void set_fullinfo_or_dark_or_blind(int _v);
	void init_pieces();
	void init_board(int _turn, char _board[MAX_LINES*MAX_COLS]);
	void print_board(FILE* _fp);
	void get_board(char _ret[128]);
	void get_board(int _side, char _ret[128]);

	void print_turn_and_moves(FILE* _fp);
	void update_moves();
	void update_moves(int _color);
	
	bool white_can_move_right(int _line, int _col);
	bool white_can_move_forward(int _line, int _col);
	bool white_can_move_left(int _line, int _col);
	bool black_can_move_right(int _line, int _col);
	bool black_can_move_forward(int _line, int _col);
	bool black_can_move_left(int _line, int _col);

	bt_move_t get_rand_move();
	bt_move_t get_rand_move(int _color);

	bool can_play(bt_move_t _m);
	bool can_simultaneous_play(bt_move_t _white_move, bt_move_t _black_move);
	void play(bt_move_t _m);
	void play(bt_move_t _w, bt_move_t _b);

	int classic_alternate_fullinfo_endgame();
	int misere_alternate_fullinfo_endgame();
	int classic_simultaneous_fullinfo_endgame();
	int misere_simultaneous_fullinfo_endgame();
	int endgame();

	double score(int _color);
	void playout(bool _log);
	double eval();
			
	void add_move(int _li, int _ci, int _lf, int _cf) {
		moves[nb_moves].line_i = _li; moves[nb_moves].col_i = _ci;
		moves[nb_moves].line_f = _lf; moves[nb_moves].col_f = _cf;
		nb_moves++;
	}
};
	
void bt_t::init(int _nbl, int _nbc) {
	if(_nbl > MAX_LINES || _nbc > MAX_COLS) {
		fprintf(stderr, "ERROR : MAX_LINES or MAX_COLS exceeded\n");
		exit(0);
	}
	nbl = _nbl; nbc = _nbc;
	classic_or_misere = 0;
	alternate_or_simultaneous = 0;
	fullinfo_or_dark_or_blind = 0;
	turn = 0;
	turn_of_last_moves_update = -1;
	color_of_last_moves_update = EMPTY;
	for(int i = 0; i < nbl; i++)
		for(int j = 0; j < nbc; j++) {
			if(i <= 1 ) {
				board[i][j] = BLACK;
			} else if(i < _nbl-2) {
				board[i][j] = EMPTY;
			} else {
				board[i][j] = WHITE;
			}
		}
	init_pieces();
	update_moves();
}
void bt_t::set_cla_or_mis(int _v) {
	classic_or_misere = _v;
}
void bt_t::set_alt_or_sim(int _v) {
	alternate_or_simultaneous = _v;
}
void bt_t::set_fullinfo_or_dark_or_blind(int _v) {
	fullinfo_or_dark_or_blind = _v;
}
void bt_t::init_pieces() {
	nb_white_pieces = 0;
	nb_black_pieces = 0;
	for(int i = 0; i < nbl; i++)
		for(int j = 0; j < nbc; j++) {
			if(board[i][j] == WHITE) {
				white_pieces[nb_white_pieces].line = i;
				white_pieces[nb_white_pieces].col = j;
				nb_white_pieces++;
			} else if(board[i][j] == BLACK) {
				black_pieces[nb_black_pieces].line = i;
				black_pieces[nb_black_pieces].col = j;
				nb_black_pieces++;
			}
		}
}
void bt_t::init_board(int _turn, char _board[MAX_LINES*MAX_COLS]) {
	nb_white_pieces = 0;
	nb_black_pieces = 0;
	turn = _turn;
	int stri = 0;
	for(int i = 0; i < nbl; i++)
		for(int j = 0; j < nbc; j++) {
			if(_board[stri] == cboard[BLACK]) {
				board[i][j] = BLACK;
				black_pieces[nb_black_pieces].line = i;
				black_pieces[nb_black_pieces].col = j;
				nb_black_pieces++;
			} else if(_board[stri] == cboard[WHITE]) {
				board[i][j] = WHITE;
				white_pieces[nb_white_pieces].line = i;
				white_pieces[nb_white_pieces].col = j;
				nb_white_pieces++;
			} else if(_board[stri] == cboard[EMPTY]) 
				board[i][j] = EMPTY;
			else if(_board[stri] == cboard[UNSEEN]) 
				board[i][j] = UNSEEN;      
			else 
				fprintf(stderr, "_str_board[%d] at %d %d = %c ?\n", stri, i, j, _board[stri]);
			stri++;
		}  
	turn_of_last_moves_update = turn-1; // to force update
	color_of_last_moves_update = EMPTY;
	update_moves();
}

void bt_t::print_board(FILE* _fp = stderr) {
#ifdef USE_COLOR
	fprintf(_fp, "   \x1B[34m");
	for(int j = 0; j < nbc; j++) {
		fprintf(_fp, "%c ", 'a'+j);
	}
	fprintf(_fp, "\x1B[0m\n");
	for(int i = 0; i < nbl; i++) {
		fprintf(_fp, "\x1B[34m%2d\x1B[0m ", (nbl-i));
		for(int j = 0; j < nbc; j++) {
			if(board[i][j] == BLACK) 
				fprintf(_fp, "\x1B[31m%c\x1B[0m ", cboard[board[i][j]]);
			else
				fprintf(_fp, "%c ", cboard[board[i][j]]);
		}
		fprintf(_fp, "\n");
	}
#else
	fprintf(_fp, "   ");
	for(int j = 0; j < nbc; j++) {
		fprintf(_fp, "%c ", 'a'+j);
	}
	fprintf(_fp, "\n");
	for(int i = 0; i < nbl; i++) {
		fprintf(_fp, "%2d ", (nbl-i));
		for(int j = 0; j < nbc; j++) {
			if(board[i][j] == BLACK) 
				fprintf(_fp, "%c ", cboard[board[i][j]]);
			else
				fprintf(_fp, "%c ", cboard[board[i][j]]);
		}
		fprintf(_fp, "\n");
	}
#endif /* USE_COLOR */
}
void bt_t::get_board(char _ret[128]) {
	int ii = 0;
	for(int i = 0; i < nbl; i++) {
		for(int j = 0; j < nbc; j++) {
			_ret[ii] = cboard[board[i][j]];
			ii++;
		}
	}
}
void bt_t::get_board(int _side, char _ret[128]) {
	if(fullinfo_or_dark_or_blind == 0) {
			get_board(_ret);
	} else if(fullinfo_or_dark_or_blind == 1) {
		for(int i = 0; i < nbl*nbc; i++) {
			_ret[i] = '?';
		}
		int wdec = nbc;
		int bdec = 1;
		char c_side = '@';
		if(_side==WHITE) {
			wdec = -nbc;
			bdec = -1;
			c_side = 'o';
		}
		int where = 0;
		for(int i = 0; i < nbl; i++) {
			for(int j = 0; j < nbc; j++) {
				if(board[i][j] == _side) {
					_ret[where] = c_side;
					if(j == 0) {
						_ret[where+wdec] = cboard[board[i+bdec][j]];
						_ret[where+wdec+1] = cboard[board[i+bdec][j+1]];
					} else if(j == (nbc-1)) {
						_ret[where+wdec] = cboard[board[i+bdec][j]];
						_ret[where+wdec-1] = cboard[board[i+bdec][j-1]];
					} else {
						_ret[where+wdec] = cboard[board[i+bdec][j]];
						_ret[where+wdec+1] = cboard[board[i+bdec][j+1]];
						_ret[where+wdec-1] = cboard[board[i+bdec][j-1]];
					}
				}
				where++;
			}
		}
	} else if(fullinfo_or_dark_or_blind == 2) {
		for(int i = 0; i < nbl*nbc; i++) {
			_ret[i] = '?';
		}
		char c_side = '@';
		if(_side==WHITE) {
			c_side = 'o';
		}
		int where = 0;
		for(int i = 0; i < nbl; i++) {
			for(int j = 0; j < nbc; j++) {
				if(board[i][j] == _side) {
					_ret[where] = c_side;
				}
				where++;
			}
		}
	}
}
void bt_t::print_turn_and_moves(FILE* _fp = stderr) {
	fprintf(_fp,"turn:%d\nmoves:", turn);
	for(int i = 0; i < nb_moves; i++) {
		moves[i].print(_fp, turn%2 == 1, nbl);
		fprintf(_fp, " ");
	}
	fprintf(_fp, "\n");
}
void bt_t::update_moves() {
	if(turn%2 == 0) update_moves(WHITE);
	else update_moves(BLACK);
}
void bt_t::update_moves(int _color) {
	if(turn_of_last_moves_update == turn && color_of_last_moves_update == _color) return; // MAJ ever done
	turn_of_last_moves_update = turn;
	color_of_last_moves_update = _color;
	nb_moves = 0;
	if(_color==WHITE) {
		for(int i = 0; i < nb_white_pieces; i++) {
			int li = white_pieces[i].line;
			int ci = white_pieces[i].col;
			if(white_can_move_right(li, ci)) add_move(li, ci, li-1, ci+1);
			if(white_can_move_forward(li, ci)) add_move(li, ci, li-1, ci);
			if(white_can_move_left(li, ci)) add_move(li, ci, li-1, ci-1);
		}
	} else if(_color == BLACK) {
		for(int i = 0; i < nb_black_pieces; i++) {
			int li = black_pieces[i].line;
			int ci = black_pieces[i].col;
			if(black_can_move_right(li, ci)) add_move(li, ci, li+1, ci+1);
			if(black_can_move_forward(li, ci)) add_move(li, ci, li+1, ci);
			if(black_can_move_left(li, ci)) add_move(li, ci, li+1, ci-1);
		}
	}
}
bool bt_t::white_can_move_right(int _line, int _col) {
	if(_line == 0) return false;
	if(_col == nbc-1) return false;
	if(board[_line-1][_col+1] != WHITE) return true;
	return false;
}
bool bt_t::white_can_move_forward(int _line, int _col) {
	if(_line == 0) return false;
	if(board[_line-1][_col] == EMPTY) return true;
	return false;
}
bool bt_t::white_can_move_left(int _line, int _col) {
	if(_line == 0) return false;
	if(_col == 0) return false;
	if(board[_line-1][_col-1] != WHITE) return true;
	return false;
}
bool bt_t::black_can_move_right(int _line, int _col) {
	if(_line == nbl-1) return false;
	if(_col == nbc-1) return false;
	if(board[_line+1][_col+1] != BLACK) return true;
	return false;
}
bool bt_t::black_can_move_forward(int _line, int _col) {
	if(_line == nbl-1) return false;
	if(board[_line+1][_col] == EMPTY) return true;
	return false;
}
bool bt_t::black_can_move_left(int _line, int _col) {
	if(_line == nbl-1) return false;
	if(_col == 0) return false;
	if(board[_line+1][_col-1] != BLACK) return true;
	return false;
}
// do not updates moves if not necessary
bt_move_t bt_t::get_rand_move() {
	update_moves();
	int r = ((int)rand())%nb_moves;
	return moves[r];
}
// always updates moves due to color
bt_move_t bt_t::get_rand_move(int _color) {
	update_moves(_color);  
	int r = ((int)rand())%nb_moves;
	return moves[r];
}
bool bt_t::can_play(bt_move_t _m) {
	int dx = abs(_m.col_f - _m.col_i);
	if(dx > 1) return false;
	int dy = abs(_m.line_f - _m.line_i);
	if(dy > 1) return false;
	if(_m.line_i < 0 || _m.line_i >= nbl) return false;
	if(_m.line_f < 0 || _m.line_f >= nbl) return false;
	if(_m.col_i < 0 || _m.col_i >= nbc) return false;
	if(_m.col_f < 0 || _m.col_f >= nbc) return false;
	int color_i = board[_m.line_i][_m.col_i];
	int color_f = board[_m.line_f][_m.col_f];
	if(color_i == EMPTY) return false;
	if(color_i == color_f) return false;
	if(alternate_or_simultaneous == 0) {
		if(turn%2==0 && color_i == BLACK) return false;
		if(turn%2==1 && color_i == WHITE) return false;
		if(_m.col_i == _m.col_f && color_f != EMPTY) return false;
	}
	return true;
}
bool bt_t::can_simultaneous_play(bt_move_t _white_move, bt_move_t _black_move) {
	if(_white_move.line_f == _black_move.line_f &&
		 _white_move.col_f == _black_move.col_f) return false;
	if(_white_move.line_f == _black_move.line_i &&
		 _white_move.line_i == _black_move.line_f &&
		 _white_move.col_f == _black_move.col_i &&
		 _white_move.col_i == _black_move.col_f) return false;
	return true;
}
void bt_t::play(bt_move_t _m) {
	int color_i = board[_m.line_i][_m.col_i];
	int color_f = board[_m.line_f][_m.col_f];
	board[_m.line_f][_m.col_f] = color_i;
	board[_m.line_i][_m.col_i] = EMPTY;
	if(color_i == WHITE) {
		for(int i = 0; i < nb_white_pieces; i++) {
			if(white_pieces[i].line == _m.line_i && white_pieces[i].col == _m.col_i) {
				white_pieces[i].line = _m.line_f;
				white_pieces[i].col = _m.col_f;
				break;
			}
		}
		if(color_f == BLACK) {
			for(int i = 0; i < nb_black_pieces; i++) {
				if(black_pieces[i].line == _m.line_f && black_pieces[i].col == _m.col_f) {
					black_pieces[i] = black_pieces[nb_black_pieces-1];
					nb_black_pieces--;
					break;
				}
			}
		}
	} else if(color_i == BLACK) {
		for(int i = 0; i < nb_black_pieces; i++) {
			if(black_pieces[i].line == _m.line_i &&
				 black_pieces[i].col == _m.col_i) {
				black_pieces[i].line = _m.line_f;
				black_pieces[i].col = _m.col_f;
				break;
			}
		}
		if(color_f == WHITE) {
			for(int i = 0; i < nb_white_pieces; i++) {
				if(white_pieces[i].line == _m.line_f &&
					 white_pieces[i].col == _m.col_f) {
					white_pieces[i] = white_pieces[nb_white_pieces-1];
					nb_white_pieces--;
					break;
				}
			}
		}
	}
	turn++;
}
void bt_t::play(bt_move_t _w, bt_move_t _b) {
	board[_w.line_i][_w.col_i] = EMPTY;
	board[_b.line_i][_b.col_i] = EMPTY;  
	int wcolor_f = board[_w.line_f][_w.col_f];
	int bcolor_f = board[_b.line_f][_b.col_f];
	board[_w.line_f][_w.col_f] = WHITE;
	board[_b.line_f][_b.col_f] = BLACK;
	for(int i = 0; i < nb_white_pieces; i++) {
		if(white_pieces[i].line == _w.line_i && white_pieces[i].col == _w.col_i) {
				white_pieces[i] = white_pieces[nb_white_pieces-1];
				nb_white_pieces--;
			break;
		}
	}
	for(int i = 0; i < nb_black_pieces; i++) {
		if(black_pieces[i].line == _b.line_i && black_pieces[i].col == _b.col_i) {
				black_pieces[i] = black_pieces[nb_black_pieces-1];
				nb_black_pieces--;
			break;
		}
	}
	if(wcolor_f == BLACK) {
		for(int i = 0; i < nb_black_pieces; i++) {
			if(black_pieces[i].line == _w.line_f && black_pieces[i].col == _w.col_f) {
				black_pieces[i] = black_pieces[nb_black_pieces-1];
				nb_black_pieces--;
				break;
			}
		}
	}
	if(bcolor_f == WHITE) {
		for(int i = 0; i < nb_white_pieces; i++) {
			if(white_pieces[i].line == _b.line_f && white_pieces[i].col == _b.col_f) {
				white_pieces[i] = white_pieces[nb_white_pieces-1];
				nb_white_pieces--;
				break;
			}
		}
	}
	white_pieces[nb_white_pieces].line = _w.line_f;
	white_pieces[nb_white_pieces].col = _w.col_f;
	nb_white_pieces++;
	black_pieces[nb_black_pieces].line = _b.line_f;
	black_pieces[nb_black_pieces].col = _b.col_f;
	nb_black_pieces++;
	turn++;
}
int bt_t::classic_alternate_fullinfo_endgame() {
	for(int i = 0; i < nbc; i++) {
		if(board[0][i] == WHITE) return WHITE;
	}
	for(int i = 0; i < nbc; i++) {
		if(board[nbl-1][i] == BLACK) return BLACK;
	}  
	if(nb_black_pieces==0) return WHITE;
	if(nb_white_pieces==0) return BLACK;
	return EMPTY;
}
int bt_t::misere_alternate_fullinfo_endgame() {
	int r = classic_alternate_fullinfo_endgame();
	if(r == BLACK) return WHITE;
	if(r == WHITE) return BLACK; 
	return EMPTY;
}
int bt_t::classic_simultaneous_fullinfo_endgame() {
	bool white_wins = false;
	bool black_wins = false;
	for(int i = 0; i < nbc; i++) {
		if(board[0][i] == WHITE) white_wins = true;
	}
	for(int i = 0; i < nbc; i++) {
		if(board[nbl-1][i] == BLACK) black_wins = true;
	}
	if(nb_black_pieces==0) white_wins = true;
	if(nb_white_pieces==0) black_wins = true;
	if(white_wins == true && black_wins == false) return WHITE;
	if(white_wins == false && black_wins == true) return BLACK;
	if(white_wins == true && black_wins == true) return DRAW;
	return EMPTY;
}
int bt_t::misere_simultaneous_fullinfo_endgame() {
	int r = classic_simultaneous_fullinfo_endgame();
	if(r == BLACK) return WHITE;
	if(r == WHITE) return BLACK; 
	if(r == DRAW) return DRAW; 
	return EMPTY;
}
int bt_t::endgame() {
	if(classic_or_misere == 0 && 
		alternate_or_simultaneous == 0 && 
		fullinfo_or_dark_or_blind == 0) return classic_alternate_fullinfo_endgame();
	if(classic_or_misere == 1 && 
		alternate_or_simultaneous == 0 && 
		fullinfo_or_dark_or_blind == 0) return misere_alternate_fullinfo_endgame();
	if(classic_or_misere == 0 && 
		alternate_or_simultaneous == 1 && 
		fullinfo_or_dark_or_blind == 0) return classic_simultaneous_fullinfo_endgame();
	if(classic_or_misere == 1 && 
		alternate_or_simultaneous == 1 && 
		fullinfo_or_dark_or_blind == 0) return misere_simultaneous_fullinfo_endgame();
	// else referee is needed
	return EMPTY;
}
double bt_t::score(int _color) {
	int state = endgame();
	if(state == EMPTY) return 0.0;
	if(_color == state) return 1.0;
	return -1.0;
}
double bt_t::eval() {
	double value = nb_white_pieces - nb_black_pieces;

	if (turn%2 == WHITE) 
	{
		for (int col = 1; col < nbc; col++)
		{
			for (int row = 1; row <= nbl; row++)
			{
        if (board[row][col] == turn%2)
        {
          // les blancs doivent atteindre 0 

          // zone support
          if (board[row+1][col]   == turn%2) 	value += (nbl-row)*0.1;				
          if (board[row+1][col+1] == turn%2) 	value += (nbl-row)*0.1;	
          if (board[row+1][col-1] == turn%2) 	value += (nbl-row)*0.1;	
          if (board[row+1][col]   == turn%2)  value += (nbl-row)*0.1;	
          if (board[row-1][col]   == turn%2) 	value += (nbl-row)*0.1;				
          if (board[row-1][col+1] == turn%2) 	value += (nbl-row)*0.1;	
          if (board[row-1][col-1] == turn%2) 	value += (nbl-row)*0.1;	
          if (board[row-1][col]   == turn%2) 	value += (nbl-row)*0.1;
          
          // zone safe
          if (board[row-1][col+1] == (turn+1)%2) 	value -= (row)*0.5;	
          if (board[row-1][col-1] == (turn+1)%2) 	value -= (row)*0.5;	

          // Zone neutre
          if (board[row+1][col]   == EMPTY)   value += 0.1;        
          if (board[row+1][col+1] == EMPTY)   value += 0.1;  
          if (board[row+1][col-1] == EMPTY)   value += 0.1;  
          if (board[row+1][col]   == EMPTY)   value += 0.1;  
          if (board[row-1][col]   == EMPTY)   value += 0.1;        
          if (board[row-1][col+1] == EMPTY)   value += 0.1;  
          if (board[row-1][col-1] == EMPTY)   value += 0.1;  
          if (board[row-1][col]   == EMPTY)   value += 0.1;

          if (row+1 == nbl-1) value += 10.0;
          if (row+1 == nbl)   value += 20.0;
        }
        else if (board[row][col] == (turn+1)%2) { 
          if (row == 1) value -= 100;
         }
			}
		}
	}
	else{
		for (int col = 1; col < nbc; col++)
		{
			for (int row = 1; row < nbl; row++)
			{
        if (board[row][col] == turn%2)
        {
          // les noires doivent atteindre nbl

          // zone support
          if (board[row+1][col]   == turn%2) 	value += row*0.1;				
          if (board[row+1][col+1] == turn%2) 	value += row*0.1;	
          if (board[row+1][col-1] == turn%2) 	value += row*0.1;	
          if (board[row+1][col]   == turn%2)  value += row*0.1;	
          if (board[row-1][col]   == turn%2) 	value += row*0.1;				
          if (board[row-1][col+1] == turn%2) 	value += row*0.1;	
          if (board[row-1][col-1] == turn%2) 	value += row*0.1;	
          if (board[row-1][col]   == turn%2) 	value += row*0.1;
          
          // zone safe
          if (board[row+1][col+1] == (turn+1)%2) 	value -= (nbl - row)*0.5;	
          if (board[row+1][col-1] == (turn+1)%2) 	value -= (nbl - row)*0.5;	

          // Zone neutre
          if (board[row+1][col]   == EMPTY)   value += 0.1;        
          if (board[row+1][col+1] == EMPTY)   value += 0.1;  
          if (board[row+1][col-1] == EMPTY)   value += 0.1;  
          if (board[row+1][col]   == EMPTY)   value += 0.1;  
          if (board[row-1][col]   == EMPTY)   value += 0.1;        
          if (board[row-1][col+1] == EMPTY)   value += 0.1;  
          if (board[row-1][col-1] == EMPTY)   value += 0.1;  
          if (board[row-1][col]   == EMPTY)   value += 0.1;

          if (row+1 == 1) value += 10.0;
          if (row+1 == 0)   value += 20.0;
        }
        else if (board[row][col] == (turn+1)%2) { 
          if (row == nbl-1) value -= 100;
        }
        
			}
		}
	}
	
	
	return value;
}
#endif /* MYBT_H */
