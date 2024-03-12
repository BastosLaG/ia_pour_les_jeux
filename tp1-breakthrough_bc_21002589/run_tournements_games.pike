#!/usr/bin/env pike

// make && pike run_tournements_games.pike

// Pour changer les joueurs il faut modifier le makefile et choisir si l'on veut ids heuristic ou rand
#define DUMP_GTP_PIPES		1

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
  string generate_move() {
    return send_command("genmove")[1];
  }
  void new_game(int _nbl, int _nbc) {
    send_command("newgame "+_nbl+" "+_nbc);
  }
  void move(string _movestr) {
    send_command("play " +_movestr);
  }
  string get_extra() {
    return send_command("extra")[1];
  }
  void quit() {
    send_command("quit");
  }
};

class btp_game {
  private btp_server p0;
  private btp_server p1;
  private int verbose;
  private int in_game_pause;

  public int nb_games;

  public string p0_name;
  public int p0_score;
  public int p0_new_win;
  public int p0_wins;

  public string p1_name;
  public int p1_score;
  public int p1_new_win;
  public int p1_wins;

  public int nb_turn;
  public int board_nbl = 0;
  public int board_nbc = 0;
  bool board_alloc = false;
  public string board; // board length = board_nbl*board_nbc

  float p0_remaining_time;
  float p1_remaining_time;

  public string output_dir = "data"; // default dir
  
  void create(string command_line_player0, string command_line_player1,
        int game_nbl, int game_nbc,
	      string new_output_dir, int _in_game_pause, int _verbose) {
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
    p0_name = command_line_player0; p0_new_win = 0; p0_wins = 0;
    p1_name = command_line_player1; p1_new_win = 0; p1_wins = 0;
    
    if(new_output_dir != "") {
      output_dir = new_output_dir;
    }
  }
  void show_endgame() {
    print_board();
    werror("(%s %d %.2f) (%s %d %.2f) ",
       p0_name, p0_score, p0_remaining_time,
       p1_name, p1_score, p1_remaining_time);
    if(p0_new_win == 1) {
      werror("=> "+p0_name+" win\n");
    } else if(p1_new_win == 1) {
      werror("=> "+p1_name+" win\n");
    } else {
      werror("=> draw game\n");
    }
  }
  void print_score(string file_name) {
    Stdio.File o = Stdio.File();
    if(!o->open(file_name,"wac")) {
        write("Failed to open file.\n");
        return;
    }
    o->write(" (%s %d %.2f) (%s %d %.2f) ",
	     p0_name, p0_score, p0_remaining_time,
	     p1_name, p1_score, p1_remaining_time);
    if(p0_new_win == 1) {
      o->write("=> "+p0_name+" win\n");
    } else if(p1_new_win == 1) {
      o->write("=> "+p1_name+" win\n");
    } else {
      o->write("=> draw game\n");
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
  bool play_move(string move) {
    if(verbose >= 1) werror("==== play_move "+move+"\n");
    if(move == "PASS") { nb_turn ++; return true; }
    int strpos = 0;
    int line_i = board_nbl-(move[0]-'0');
    int col_i = move[1]-'a';
    int line_f = board_nbl-(move[2]-'0');
    int col_f = move[3]-'a';
    if(verbose >= 2) werror("==== play at "+line_i+" "+col_i+" "+line_f+" "+col_f+"\n");
    if(line_i < 0 || line_f < 0) return false;
    if(line_i >= board_nbl || line_f >= board_nbl) return false;
    if(col_i < 0 || col_f < 0) return false;
    if(col_i >= board_nbc || col_f >= board_nbc) return false;
    if(abs(line_f-line_i) > 1) return false;
    if(abs(col_f-col_i) > 1) return false;
    if(nb_turn%2==0) {
      board[line_i*board_nbc+col_i] = '.';
      board[line_f*board_nbc+col_f] = 'o';
    } else {
      board[line_i*board_nbc+col_i] = '.';
      board[line_f*board_nbc+col_f] = '@';
    }
    nb_turn ++;
    return true;
  }
  bool endgame() {
    for(int i = 0; i < board_nbc; i++)
      if(board[i] == 'o') return true;
    for(int i = (board_nbl-1)*board_nbc; i < board_nbl*board_nbc; i++)
      if(board[i] == '@') return true;
    int nb_white = 0;
    int nb_black = 0;
    for(int i = 0; i < board_nbl; i++) {
      for(int j = 0; j < board_nbc; j++) {
        if(board[i*board_nbc+j] == '@') nb_white++;
        if(board[i*board_nbc+j] == 'o') nb_black++;
      }
    }
    if(nb_white == 0) return true;
    if(nb_black == 0) return true;
    return false;
  }
  int count_pawn_on_board() {
    int ret = 0;
    for(int i = 0; i < board_nbl*board_nbc; i++) {
      if(board[i] != '.') ret += 1;
    }
    return ret;
  }
  void play() {
    if (verbose >= 2) werror("\nBeginning a new game.\n");
    p0_new_win = 0;
    p1_new_win = 0;
    p0_score = 0; 
    p1_score = 0;
    p0->new_game(board_nbl, board_nbc);
    p1->new_game(board_nbl, board_nbc);
    init_board();

    // perform a match
    string p0_move = "";
    string p1_move = "";
    while(true) {
      if(verbose >= 1) print_board();
      array(int) Ti = System.gettimeofday();
      if(verbose >= 2) werror("calling generate_move\n");
      p0_move = p0->generate_move();
      if(verbose >= 2) werror("P0_move received : "+p0_move+"\n");
      array(int) Tf = System.gettimeofday();
      float ms = (float)((Tf[0] - Ti[0]))+(float)(Tf[1] - Ti[1])/1000000;
      p0_remaining_time -= ms;
      if(p0_remaining_time < 0.0) {
        p0_new_win = 0; p1_new_win = 1;
        werror(" ===> "+p0_name+" time exceeded\n");
        print_board();
        werror(" ===> "+p1_name+" WIN\n");
        break;
      }
      if(play_move(p0_move) == false) {
        p0_new_win = 0; p1_new_win = 1;
        werror(" ===> "+p0_name+" try to play "+p0_move+"\n");
        print_board();
        werror(" ===> "+p1_name+" WIN\n");
        break;
      } else {
        if(verbose >= 2) {
          werror("==== ok\n");
        }
      }
      p1->move(p0_move);
      if(endgame()) {
        if(verbose >= 2) {
          werror("=== endgame DETECTED\n");
                  print_board();
        }
        p0_new_win = 1;
        p0_score = count_pawn_on_board();
        p1_new_win = 0;
        p1_score = -count_pawn_on_board();
        break;
      }
      if(p0_move == "PASS" && p1_move == "PASS") {
        if(verbose >= 2) werror("=== all players PASS DETECTED\n");
        break;
      }
      if(verbose >= 2) print_board();
      Ti = System.gettimeofday();
      p1_move = p1->generate_move();
      if(verbose >= 2) werror("P1_move received : "+p1_move+"\n");
      Tf = System.gettimeofday();
      ms = (float)((Tf[0] - Ti[0]))+(float)(Tf[1] - Ti[1])/1000000;
      p1_remaining_time -= ms;
      if(p1_remaining_time < 0.0) {
        p1_new_win = 0; p0_new_win = 1;
        werror(" ===> "+p1_name+" time exceeded\n");
        print_board();
        werror(" ===> "+p0_name+" WIN\n");
        break;
      }
      if(play_move(p1_move) == false) {
        p1_new_win = 0; p0_new_win = 1;
        werror(" ===> "+p1_name+" try to play "+p1_move+"\n");
        print_board();
        werror(" ===> "+p0_name+" WIN\n");
        break;
      } else {
        if(verbose >= 2) {
          werror("==== ok\n");
        }
      }
      p0->move(p1_move);
      if(in_game_pause > 0)
        sleep(in_game_pause);
      if(endgame()) {
        if(verbose >= 2) {
          werror("=== endgame DETECTED\n");
          print_board();
        }
        p1_new_win = 1;
        p1_score = count_pawn_on_board();
        p0_new_win = 0;
        p0_score = -count_pawn_on_board();
        break;
      }
      if(p0_move == "PASS" && p1_move == "PASS") {
        if(verbose >= 2) werror("=== all players PASS DETECTED\n");
        break;
      }
    }
  }
  void game_stats() {
    werror("game_length: "+p0->get_extra()+"\n");
  }
  void finalize() {
    p0->quit(); p1->quit(); 
  }
}

void run_many_games(btp_game game, int _nb_games_to_play, int verbose) {
  game->nb_games = 0;
  for (int k = 0; k < _nb_games_to_play; k++) {
    game->play();
    //werror("nb_turn: "+game->nb_turn+"\n");
    //game->game_stats();
    if(game->p0_new_win == 1) {
      game->show_endgame();
      werror("================= %s WIN\n",game->p0_name);
      game->p0_wins ++;
    } 
    if(game->p1_new_win == 1) {
      game->show_endgame();
      werror("================= %s WIN\n",game->p1_name);
      game->p1_wins ++;
    } 
    if(game->p0_new_win == 0 && game->p1_new_win == 0) {
      werror("================= noone WIN\n");
    }
    game->nb_games ++;
    game->print_score(game->output_dir+"/scores.txt");
  }
  game->finalize();
}

int main() {
  int i;
  string winner;
  array(string) contestants = ({ "./Player1", "./Player2", "./Player3", "./Player4", "./Player5", "./Player6", "./Player7", "./Player8" });
  array(string) semis = ({});
  array(string) finale = ({});

  for (i=0;i<7;i+=2){
      btp_game game = btp_game(contestants[i],contestants[i+1], 6, 4, "", 0, 0);
      if (game) {
          run_many_games(game, 1,0);
      }
      if(game->p0_new_win == 1) {
          winner = game->p0_name;
          semis += ({winner});
      } else if(game->p1_new_win == 1) {
          winner = game->p1_name;
          semis += ({winner});
      }
  }
  
  for (i=0;i<3;i+=2){
      btp_game game = btp_game(semis[i],semis[i+1], 6, 4, "", 0, 0);
      if (game) {
          run_many_games(game, 1,0);
      }
      if(game->p0_new_win == 1) {
          winner = game->p0_name;
          finale += ({winner});
      } else if(game->p1_new_win == 1) {
          winner = game->p1_name;
          finale += ({winner});
      }
  }

  btp_game game = btp_game(finale[0],finale[1], 6, 4, "", 0, 0);
    if (game) {
      run_many_games(game, 1,0);
    }
    if(game->p0_new_win == 1) {
        winner = game->p0_name;
    } else if(game->p1_new_win == 1) {
      winner = game->p1_name;
    }
    for(i=0;i<15;i++){
      if(i%2==0){
        int eliminated = 1;
        for (int j=0; j < sizeof(semis); j++){
          if (contestants[i/2] == semis[j]) {
            eliminated = 0;
            break;
          }
        }
        if (eliminated) {
            werror("\x1b[31m%s\x1b[0m\n", contestants[i/2]);
        } else {
            werror("%s\n", contestants[i/2]);
        }
      }
      if(i%4==0){
        int eliminated = 1;
        for (int j=0; j < sizeof(finale); j++){
          if (semis[i/4] == finale[j]) {
              eliminated = 0;
              break;
          }
        }
        if (eliminated) {
          werror("\t\x1b[31m%s\x1b[0m\n", semis[i/4]);
        } else {
          werror("\t%s\n", semis[i/4]);
        }
      }
      if(i==3){
        if (finale[0]!=winner){
          werror("\t\t\x1b[31m%s\x1b[0m\n", finale[0]);
        }
        else{
          werror("\t\t%s\n", finale[0]);
        }
      }
      if(i==10){
        if (finale[1]!=winner){
          werror("\t\t\x1b[31m%s\x1b[0m\n", finale[1]);
        }
        else{
          werror("\t\t%s\n", finale[1]);
        }
      }
      if(i==7){
      werror("\t\t\t\x1b[32m%s\x1b[0m\n",winner);
      }
  }
  return 0;
}
