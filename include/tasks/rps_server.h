#ifndef _rps_server_h_
#define _rps_server_h_ 1

#include "tasks/first_user_task.h"
#include "tasks/task_functions.h"
#include "system_calls.h"
#include "rpi.h"
#include "util.h"

enum RPSState {
    RPS_ROUND_OVER = 0,
    RPS_WIN,
    RPS_LOSS,
    RPS_TIE,
    RPS_READY,
    RPS_WAIT_FOR_PARTNER,
    RPS_PARTNER_QUIT,
};

enum RPSMoves {
    RPS_NOT_PLAYED = 0,
    RPS_ROCK,
    RPS_PAPER,
    RPS_SCISSORS,
};

enum RPSCommands {
    RPS_NONE = 0,
    RPS_SIGNUP,
    RPS_PLAY,
    RPS_QUIT,
};

class RpsServer {
public:
    static const int RPS_CLIENT_REQUEST_LEN = 4;
    static const int NUM_PLAYERS = 10;    
    
    int waiting_player;
    int players[NUM_PLAYERS];
    int moves[NUM_PLAYERS];

    void init_rps_server();
    int getWinner(int player1, int player2);

};

#endif
