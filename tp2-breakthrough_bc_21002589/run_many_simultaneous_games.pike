#!/usr/bin/env pike

// pike run_many_simultaneous_games.pike -f ./rand_player -s ./rand_player -v 1 -p 1 -l 5 -c 3 -i 0 -m 0

//  "Options:\n"
//  "  -n, --number=NB_GAMES         the number of games to play\n"
//  "  -l, --nbl=NB_LINES            the number of lines on the board\n"
//  "  -c, --nbc=NB_COLS             the number of cols on the board\n"
//  "  -f, --first=COMMAND_LINE\n"
//  "  -s, --second=COMMAND_LINE     command lines to run the two engines with.\n\n"
//  "  -o, --outputdir=OUTPUT_DIRECTORY (default ouput is data)\n"
//  "      --help                    display this help and exit.\n"
//  "  -v, --verbose=LEVEL           1 - print moves, 2 and higher - draw boards.\n"
//  "  -p, --pause=SECONDS           1 - sleep(1), 2 and more - sleep(2 and more).\n"
//  "  -i, --init=SEED               to set the initial random seed.\n"
//  "  -m, --mode=MODE_ID            0 - classic, 1 - misere.\n";

#define DUMP_GTP_PIPES		0

// communication protocol for a breakthrough game (as seen by the referee)
// !!! simultaneous moves game mode !!!
// [names, new game, cla or mis]
// ... 
// [referee ---> j0 ] mode sim
// [j0 ---> referee ] =
// [referee ---> j1 ] mode sim
// [j1 ---> referee ] =
// ...
// [referee ---> j0 ] genmove o
// [j0 ---> referee ] = MOVE0
// [referee ---> j1 ] genmove @
// [j1 ---> referee ] = MOVE1
// [referee checks MOVE0 and MOVE1 on its board]
// [referee checks conficts between MOVE0 and MOVE1]
// [declares j1's win in case of illegal move from j0]
// [declares j0's win in case of illegal move from j1]
// [referee plays MOVE0 and MOVE1 on its board]
// [if without conflicts, then it sends moves to each player]
// [referee ---> j0 ] play MOVE0 MOVE1
// [j0 ---> referee ] = 
// [referee ---> j1 ] play MOVE0 MOVE1
// [j1 ---> referee ] = 
// ...
// [if endgame or out of time are detected by the referee ]
// [referee ---> j0 ] quit
// [j0 ---> referee ] = 
// [referee ---> j1 ] quit
// [j1 ---> referee ] =

// specific simultaneous games playmove
#define NO_MOVE_ERROR 0
#define WHITE_MOVE_ERROR 1  // wrong white move
#define BLACK_MOVE_ERROR 2  // wrong black move
#define BOTH_MOVE_ERROR 3   // wrong white and wrong black moves
#define NULL_MOVE_ERROR 4   // collision between moves

// specific simultaneous games endgame
#define NO_ENDGAME 5
#define WHITE_WIN_ENDGAME 6
#define BLACK_WIN_ENDGAME 7
#define DRAW_ENDGAME 8

class btp_server { // breakthrough text protocol server
  int server_is_up;
  private Stdio.File file_out; // reading without buffer (stream mode)
  private Stdio.FILE file_in;  // reading with buffer (line per line mode)
  string command_line;
  string|int engine_name;      // just for fun

  void create(string _command_line) {
    file_out = Stdio.File();
    file_in = Stdio.FILE();
    command_line = _command_line;
    array error = catch { 
	    Process.create_process(command_line / " ",
			       ([ "stdin" : file_out->pipe(),
				  "stdout" : file_in->pipe() ])); };
    if (error) {
      werror(error[0]); werror("Command line was `%s'.\n", command_line);
      destruct(this_object());
    } else {
      array error = catch {
	      engine_name = get_name();
        server_is_up = 1;
      };
      if (error) {
        werror("Engine `%s' crashed at startup.\nPerhaps command line is wrong.\n", command_line);
	      destruct(this_object());
      }
    }
  }
  
  array send_command(string command) {
#if DUMP_GTP_PIPES
    werror("[%s] %s\n", engine_name ? engine_name : "", command);
#endif
    command = String.trim_all_whites(command);
    sscanf(command, "%[0-9]", string id);
    if (command[0] == '#' || command == id) return ({ 0, "" });
    file_out->write("%s\n", command);
    string response = file_in->gets();
    if (!response) {
      server_is_up = 0;
      error("Engine `%s' playing crashed!", command_line);
    }
#if DUMP_GTP_PIPES
    werror("%s\n", response);
#endif
    array result;
    int id_length = strlen(id);
    if (response && response[..id_length] == "=" + id)
      result = ({ 0, response[id_length + 1 ..] });
    else if (response && response[..id_length] == "?" + id)
      result = ({ 1, response[id_length + 1 ..] });
    else
      result = ({ -1, response });
    result[1] = String.trim_all_whites(result[1]);
    while (1) {
      response = file_in->gets();
#if DUMP_GTP_PIPES
      werror("%s\n", response);
#endif
      if (response == "") {
        if (result[0] < 0) {
          werror("Warning, unrecognized response to command `%s':\n", command);
          werror("%s\n", result[1]);
        }
        return result;
      }
      result[1] += "\n" + response;
    }
  }
  string get_name() {
    return send_command("name")[1];
  }
  string get_genmove(string _turn) {
    return send_command("genmove "+_turn)[1];
  }
  void set_newgame(int _nbl, int _nbc) {
    send_command("newgame "+_nbl+" "+_nbc);
  }
  void set_play(string _move0str, string _move1str) {
    send_command("play " +_move0str+" "+_move1str);
  }
  string get_extra() {
    return send_command("extra")[1];
  }
  void set_mode(string _mode) {
    send_command("mode " +_mode);
  }
  void set_seed(int _seed) {
    send_command("seed " +_seed);
  }
  void set_quit() {
    send_command("quit");
  }
};

class btp_game {
  private btp_server p0;
  private btp_server p1;  
  private int verbose;
  private int in_game_pause;

  public int init_seed;
  public int mode_id;
  public int nb_games;

  public int gamestatus_id;

  public string p0_name;
  public float p0_score;   // in simultaneous game, draw implies half win
  public float p0_wins;

  public string p1_name;
  public float p1_score;
  public float p1_wins;

  public int nb_turn;
  public int board_nbl = 0;
  public int board_nbc = 0;
  bool board_alloc = false;
  public string board; // board length = board_nbl*board_nbc

  float p0_remaining_time;
  float p1_remaining_time;

  public string output_dir = "data"; // default dir
  
  void create(string command_line_player0, string command_line_player1,
        int _init_seed, int _mode_id, 
        int game_nbl, int game_nbc,
	      string new_output_dir, int _in_game_pause, int _verbose) {
    init_seed = _init_seed;
    mode_id = _mode_id;
    in_game_pause = _in_game_pause;
    verbose = _verbose;
    p0 = btp_server(command_line_player0);
    if (p0) p1 = btp_server(command_line_player1);
    if (!p0 || !p1) {
      werror("!p0 || !p1"); finalize(); exit(0);
    }
    board_nbl = game_nbl;
    board_nbc = game_nbc;
    nb_games = 0; 
    p0_name = command_line_player0; p0_wins = 0.0;
    p1_name = command_line_player1; p1_wins = 0.0;
    
    // to have different random seeds
    p0->set_seed(init_seed); 
    p1->set_seed(init_seed+1); 

    if(new_output_dir != "") {
      output_dir = new_output_dir;
    }
  }
  void show_endgame() {
    print_board();
    werror("(%s %.1f %.2f) (%s %.1f %.2f) ",
       p0_name, p0_score, p0_remaining_time,
       p1_name, p1_score, p1_remaining_time);
    if(gamestatus_id == WHITE_WIN_ENDGAME) {
      werror("=> "+p0_name+" win\n");
    } else if(gamestatus_id == BLACK_WIN_ENDGAME) {
      werror("=> "+p1_name+" win\n");
    } else if(gamestatus_id == DRAW_ENDGAME) {
      werror("=> draw game\n");
    } else if(gamestatus_id == NULL_MOVE_ERROR) {
      werror("=> null game\n");
    } else if(gamestatus_id == WHITE_MOVE_ERROR) {
      werror("=> "+p1_name+" win (white move error)\n");
    } else if(gamestatus_id == BLACK_MOVE_ERROR) {
      werror("=> "+p0_name+" win (black move error)\n");
    } else if(gamestatus_id == BOTH_MOVE_ERROR) {
      werror("=> null game (both move error)\n");
    }
  }
  void init_mode() {
    if(mode_id == 0) {
      p0->set_mode("cla");
      p1->set_mode("cla");
    } else if(mode_id == 1) {
      p0->set_mode("mis");
      p1->set_mode("mis");
    }
    p0->set_mode("sim");
    p1->set_mode("sim");
  }
  void print_score(string file_name) {
    Stdio.File o = Stdio.File();
    if(!o->open(file_name,"wac")) {
        write("Failed to open file.\n");
        return;
    }
    o->write(" (%s %.1f %.2f) (%s %.1f %.2f) ",
	     p0_name, p0_score, p0_remaining_time,
	     p1_name, p1_score, p1_remaining_time); 
    if(gamestatus_id == WHITE_WIN_ENDGAME) {
      o->write("=> "+p0_name+" win\n");
    } else if(gamestatus_id == BLACK_WIN_ENDGAME) {
      o->write("=> "+p1_name+" win\n");
    } else if(gamestatus_id == DRAW_ENDGAME) {
      o->write("=> draw game\n");
    } else if(gamestatus_id == NULL_MOVE_ERROR) {
      o->write("=> null game\n");
    } else if(gamestatus_id == WHITE_MOVE_ERROR) {
      o->write("=> "+p1_name+" win (white move error)\n");
    } else if(gamestatus_id == BLACK_MOVE_ERROR) {
      o->write("=> "+p0_name+" win (black move error)\n");
    } else if(gamestatus_id == BOTH_MOVE_ERROR) {
      o->write("=> null game (both move error)\n");
    }
    o->close();
  }
  // @ is black player and o is white player
  void init_board() {
    nb_turn = 0;
    p0_remaining_time = 24.0;
    p1_remaining_time = 24.0;
    if(board_alloc == false) {
      for(int i = 0; i < board_nbl*board_nbc; i++)
    	  board = board+".";
      board_alloc = true;
    } else {
      for(int i = 0; i < board_nbl*board_nbc; i++)
	      board[i] = '.';
    }
    for(int i = 0; i < 2*board_nbc; i++)
      board[i] = '@';
    for(int i = (board_nbl-2)*board_nbc; i < board_nbl*board_nbc; i++)
      board[i] = 'o';
  }
  void print_board() {
    bool color_print = false;
    if(color_print) {
      werror("nb_turn: %d   timers : \x1b[31m%.2f\x1b[0m : %.2f\n", 
	     nb_turn, p0_remaining_time, p1_remaining_time);
    } else {
      werror("nb_turn: %d   timers : %.2f : %.2f\n", 
	     nb_turn, p0_remaining_time, p1_remaining_time);
    }
    for(int i = 0; i < board_nbl; i++) {
      werror(""+(board_nbl-i)+" ");
      for(int j = 0; j < board_nbc; j++) {
      	if(color_print) {
          if(board[i*board_nbc+j] == '@') {
            werror("\x1b[31m%c\x1b[0m ", board[i*board_nbc+j]);
          } else {
            werror("%c ",board[i*board_nbc+j]);
          }
        } else {
          werror("%c ",board[i*board_nbc+j]);
        }
      }
      werror("\n");
    }
    werror("  ");
    for(int j = 0; j < board_nbc; j++)
      werror("%c ", 'a'+j);
    werror("\n");
  }

  int play_move(string move0, string move1) {
    if(verbose >= 1) werror("==== play_move "+move0+" "+move1+"\n");
    bool white_ok = true;
    // check white move
    int wline_i = board_nbl-(move0[0]-'0');
    int wcol_i = move0[1]-'a';
    int wline_f = board_nbl-(move0[2]-'0');
    int wcol_f = move0[3]-'a';
    if(verbose >= 2) werror("==== white move at "+wline_i+" "+wcol_i+" "+wline_f+" "+wcol_f+"\n");
    if(wline_i < 0 || wline_f < 0) white_ok=false;
    if(wline_i >= board_nbl || wline_f >= board_nbl) white_ok=false;
    if(wcol_i < 0 || wcol_f < 0) white_ok=false;
    if(wcol_i >= board_nbc || wcol_f >= board_nbc) white_ok=false;
    if(abs(wline_f-wline_i) > 1) white_ok=false;
    if(abs(wcol_f-wcol_i) > 1) white_ok=false;
    if(board[wline_i*board_nbc+wcol_i] == board[wline_f*board_nbc+wcol_f]) white_ok=false;

    // check black move
    bool black_ok = true;
    int bline_i = board_nbl-(move1[0]-'0');
    int bcol_i = move1[1]-'a';
    int bline_f = board_nbl-(move1[2]-'0');
    int bcol_f = move1[3]-'a';
    if(verbose >= 2) werror("==== black move at "+bline_i+" "+bcol_i+" "+bline_f+" "+bcol_f+"\n");
    if(bline_i < 0 || bline_f < 0) black_ok=false;
    if(bline_i >= board_nbl || bline_f >= board_nbl) black_ok=false;
    if(bcol_i < 0 || bcol_f < 0) black_ok=false;
    if(bcol_i >= board_nbc || bcol_f >= board_nbc) black_ok=false;
    if(abs(bline_f-bline_i) > 1) black_ok=false;
    if(abs(bcol_f-bcol_i) > 1) black_ok=false;
    if(board[bline_i*board_nbc+bcol_i] == board[bline_f*board_nbc+bcol_f]) black_ok=false;

    if(white_ok==false && black_ok) return WHITE_MOVE_ERROR;
    if(white_ok && black_ok==false) return BLACK_MOVE_ERROR;
    if(white_ok==false && black_ok==false) return BOTH_MOVE_ERROR;

    // check white and black moves together
    if(wline_f == bline_f && wcol_f == bcol_f) return NULL_MOVE_ERROR;
    if(wline_f == bline_i && wline_i == bline_f &&
      wcol_f == bcol_i && wcol_i == bcol_f) return NULL_MOVE_ERROR;

    board[wline_i*board_nbc+wcol_i] = '.';
    board[bline_i*board_nbc+bcol_i] = '.';
    board[wline_f*board_nbc+wcol_f] = 'o';
    board[bline_f*board_nbc+bcol_f] = '@';

    nb_turn ++;
    return NO_MOVE_ERROR;
  }
  int classic_endgame() {
    bool white_wins = false;
    bool black_wins = false;
    for(int i = 0; i < board_nbc; i++) {
      if(board[i] == 'o') white_wins = true;
    }
    for(int i = (board_nbl-1)*board_nbc; i < board_nbl*board_nbc; i++) {
      if(board[i] == '@') black_wins = true;
    }
    int nb_white = 0;
    int nb_black = 0;
    for(int i = 0; i < board_nbl; i++) {
      for(int j = 0; j < board_nbc; j++) {
        if(board[i*board_nbc+j] == '@') nb_white++;
        if(board[i*board_nbc+j] == 'o') nb_black++;
      }
    }    
    if(nb_black==0) white_wins = true;
    if(nb_white==0) black_wins = true;

    if(white_wins == true && black_wins == false) return WHITE_WIN_ENDGAME;
    if(white_wins == false && black_wins == true) return BLACK_WIN_ENDGAME;
    if(white_wins == true && black_wins == true) return DRAW_ENDGAME;
    return NO_ENDGAME;
  }
  int count_pawn_on_board() {
    int ret = 0;
    for(int i = 0; i < board_nbl*board_nbc; i++) {
      if(board[i] != '.') ret += 1;
    }
    return ret;
  }
  void play() {
    int nb_null_moves = 0;
    if (verbose >= 2) werror("\nBeginning a new game.\n");
    p0_score = 0.0; 
    p1_score = 0.0;
    p0->set_newgame(board_nbl, board_nbc);
    p1->set_newgame(board_nbl, board_nbc);
    init_mode();
    init_board();

    // perform a match
    string p0_move = "";
    string p1_move = "";
    while(true) {
      if(verbose >= 1) print_board();
      array(int) Ti = System.gettimeofday();
      if(verbose >= 2) werror("calling get_genmove\n");
      p0_move = p0->get_genmove("o");
      if(verbose >= 2) werror("P0_move received : "+p0_move+"\n");
      array(int) Tf = System.gettimeofday();
      float ms = (float)((Tf[0] - Ti[0]))+(float)(Tf[1] - Ti[1])/1000000;
      p0_remaining_time -= ms;
      if(p0_remaining_time < 0.0) {
        gamestatus_id = BLACK_WIN_ENDGAME;
        werror(" ===> "+p0_name+" time exceeded\n");
        print_board();
        break;
      }
      Ti = System.gettimeofday();
      if(verbose >= 2) werror("calling get_genmove\n");
      p1_move = p1->get_genmove("@");
      if(verbose >= 2) werror("P1_move received : "+p1_move+"\n");
      Tf = System.gettimeofday();
      ms = (float)((Tf[0] - Ti[0]))+(float)(Tf[1] - Ti[1])/1000000;
      p1_remaining_time -= ms;
      if(p1_remaining_time < 0.0) {
        gamestatus_id = WHITE_WIN_ENDGAME;
        werror(" ===> "+p1_name+" time exceeded\n");
        print_board();
        break;
      }
      gamestatus_id = play_move(p0_move,p1_move);
      if(gamestatus_id == WHITE_MOVE_ERROR) {
        werror(" ===> "+p0_name+" white_mode_error "+p0_move+"\n");
        print_board();
        break;
      } else if(gamestatus_id == BLACK_MOVE_ERROR) {
        werror(" ===> "+p1_name+" black_move_error "+p1_move+"\n");
        print_board();
        break;
      } else if(gamestatus_id == BOTH_MOVE_ERROR) {
        werror(" ===> "+p0_name+" move_error "+p0_move+"\n");
        werror(" ===> "+p1_name+" move_error "+p1_move+"\n");
        print_board();
        break;
      } else if(gamestatus_id == NULL_MOVE_ERROR) {
        nb_null_moves++;
        if(verbose >= 1) werror(" ===> "+p0_name+" null_move_error "+p0_move+"\n");
        if(verbose >= 1) werror(" ===> "+p1_name+" null_move_error "+p1_move+"\n");
        if(nb_null_moves >= 3) {
          break;
        }
      } else {
        p0->set_play(p0_move,p1_move);
        p1->set_play(p0_move,p1_move);
        if(verbose >= 2) {
          werror("==== ok\n");
        }
      }

      if(in_game_pause > 0)
        sleep(in_game_pause);
      if(gamestatus_id == NO_MOVE_ERROR) {
        gamestatus_id = classic_endgame();
        if(gamestatus_id == WHITE_WIN_ENDGAME) {
          if(verbose >= 2) {
            werror("=== endgame DETECTED\n"); print_board();
          }
          if(mode_id == 0) { // classic
            p0_score = 1.0*count_pawn_on_board();
            p1_score = -1.0*count_pawn_on_board();
            break;
          } else if(mode_id == 1) { // misere
            gamestatus_id = BLACK_WIN_ENDGAME;
            p0_score = -1.0*count_pawn_on_board();
            p1_score = 1.0*count_pawn_on_board();
            break;
          }
        } else if(gamestatus_id == BLACK_WIN_ENDGAME) {
          if(verbose >= 2) {
            werror("=== endgame DETECTED\n"); print_board();
          }
          if(mode_id == 0) { // classic
            p0_score = -1.0*count_pawn_on_board();
            p1_score = 1.0*count_pawn_on_board();
            break;
          } else if(mode_id == 1) { // misere
            gamestatus_id = WHITE_WIN_ENDGAME;
            p0_score = 1.0*count_pawn_on_board();
            p1_score = -1.0*count_pawn_on_board();
            break;
          }
        } else if(gamestatus_id == DRAW_ENDGAME) {
          if(verbose >= 2) {
            werror("=== endgame DETECTED\n"); print_board();
          }
          if(mode_id == 0) { // classic
            p0_score = -count_pawn_on_board()*0.5;
            p1_score = count_pawn_on_board()*0.5;
            break;
          } else if(mode_id == 1) { // misere
            p0_score = count_pawn_on_board()*0.5;
            p1_score = -count_pawn_on_board()*0.5;
            break;
          }
        }
      }
    }
  }
  void game_stats() {
    werror("game_length: "+p0->get_extra()+"\n");
  }
  void finalize() {
    p0->set_quit(); p1->set_quit(); 
  }
}

void run_many_games(btp_game game, int _nb_games_to_play, int verbose) {
  game->nb_games = 0;  
  for (int k = 0; k < _nb_games_to_play; k++) {
    game->gamestatus_id = NO_MOVE_ERROR;
    game->play();
    //werror("nb_turn: "+game->nb_turn+"\n");
    //game->game_stats();
    if(game->gamestatus_id == WHITE_WIN_ENDGAME) {
      game->show_endgame();
      werror("================= player1 WIN\n");
      game->p0_wins+=1.0;
    } 
    else if(game->gamestatus_id == BLACK_WIN_ENDGAME) {
      game->show_endgame();
      werror("================= player2 WIN\n");
      game->p1_wins+=1.0;
    } 
    else if(game->gamestatus_id == DRAW_ENDGAME) {
      werror("================= DRAW game\n");
      game->p0_wins+=0.5;
      game->p1_wins+=0.5;
    }
    else if(game->gamestatus_id == WHITE_MOVE_ERROR) {
      werror("================= WHITE MOVE ERROR game\n");
      game->p1_wins+=1.0;
    }
    else if(game->gamestatus_id == BLACK_MOVE_ERROR) {
      werror("================= BLACK MOVE ERROR game\n");
      game->p0_wins+=1.0;
    }
    else if(game->gamestatus_id == BOTH_MOVE_ERROR) {
      werror("================= BOTH MOVE ERROR game\n");
    }
    else if(game->gamestatus_id == NULL_MOVE_ERROR) {
      werror("================= NULL MOVE ERROR game\n");
      game->p0_wins+=0.5;
      game->p1_wins+=0.5;
    }
    game->nb_games ++;
    game->print_score(game->output_dir+"/scores.txt");
  }
  game->finalize();
}

string help_message =
  "Usage: %s [OPTION]... [FILE]...\n\n"
  "Runs one or many matches between two programs text protocol engines.\n"
  "`--white' and `--black' options are mandatory.\n\n"
  "Options:\n"
  "  -n, --number=NB_GAMES         the number of games to play\n"
  "  -l, --nbl=NB_LINES            the number of lines on the board\n"
  "  -c, --nbc=NB_COLS             the number of cols on the board\n"
  "  -f, --first=COMMAND_LINE\n"
  "  -s, --second=COMMAND_LINE     command lines to run the two engines with.\n\n"
  "  -o, --outputdir=OUTPUT_DIRECTORY (default ouput is data)\n"
  "      --help                    display this help and exit.\n"
  "  -v, --verbose=LEVEL           1 - print moves, 2 and higher - draw boards.\n"
  "  -p, --pause=SECONDS           1 - sleep(1), 2 and more - sleep(2 and more).\n"
  "  -i, --init=SEED               to set the initial random seed.\n"
  "  -m, --mode=MODE_ID            0 - classic, 1 - misere.\n";

int main(int argc, array(string) argv) {
  string hint = sprintf("Try `%s --help' for more information.\n",
			basename(argv[0]));
  if (Getopt.find_option(argv, UNDEFINED, "help")) {
    write(help_message, basename(argv[0]));
    return 0;
  }
  string str_p0 = Getopt.find_option(argv, "f", "first", UNDEFINED, "");
  if (str_p0 == "") {
    werror("First player is not specified.\n" + hint);
    return 1;
  }
  string str_p1 = Getopt.find_option(argv, "s", "second", UNDEFINED, "");
  if (str_p1 == "") {
    werror("Second player is not specified.\n" + hint);
    return 1;
  }
  string str_nb_games = Getopt.find_option(argv, "n", "games", UNDEFINED, "");
  string str_nbl = Getopt.find_option(argv, "l", "nbl", UNDEFINED, "");
  string str_nbc = Getopt.find_option(argv, "c", "nbc", UNDEFINED, "");
  string str_output_dir = Getopt.find_option(argv, "o", "outputdir", UNDEFINED, "");
  int verbose = (int) Getopt.find_option(argv, "v", "verbose", UNDEFINED, "0");
  int in_game_pause = (int) Getopt.find_option(argv, "p", "pause", UNDEFINED, "0");
  int init_seed = (int) Getopt.find_option(argv, "i", "init", UNDEFINED, "0");
  int mode_id = (int) Getopt.find_option(argv, "m", "mode", UNDEFINED, "0");

  int game_nbl = 8; // default
  int game_nbc = 8; // default
  int nb_games = 1; // default
  if (str_nbl != "") {
    sscanf(str_nbl, "%d", game_nbl);
    if(game_nbl <= 0) game_nbl = 8;
  }
  if (str_nbc != "") {
    sscanf(str_nbc, "%d", game_nbc);
    if(game_nbc <= 0) game_nbc = 8;
  }
  if (str_nb_games != "") {
    sscanf(str_nb_games, "%d", nb_games);
    if(nb_games <= 0) nb_games = 1;
  }

  btp_game game = btp_game(str_p0, str_p1, init_seed, mode_id, game_nbl, game_nbc, str_output_dir, in_game_pause, verbose);
  if (game) {
    run_many_games(game, nb_games, verbose);
  }
  return 0;
}
