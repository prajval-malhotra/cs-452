#include "tasks/rps_server.h"
#include "system_timer.h"

void RpsServer::init_rps_server() {
    // initialize players and moves
    waiting_player = -1;
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        this->players[i] = -1;
        this->moves[i] = RPS_NOT_PLAYED;
    }
}
    
int RpsServer::getWinner(int player1, int player2) {
    if(moves[player1] == moves[player2]) return 0;
    else if((moves[player1] == RPS_ROCK     && moves[player2] == RPS_SCISSORS) ||
            (moves[player1] == RPS_PAPER    && moves[player2] == RPS_ROCK)     ||
            (moves[player1] == RPS_SCISSORS && moves[player2] == RPS_PAPER)) return player1;
    
    return player2;
}


void rps_server() {
    uart_printf(CONSOLE, "[RPS]: Starting RPS server...\r\n");

    volatile int retval = -1;
    while(retval < 0) retval = RegisterAs("rps_server");
    
    RpsServer rps_server;
    rps_server.init_rps_server();
    
    int client_tid;
    char msg[2];
    char reply[2];
    volatile int msg_len = 2;
    volatile int reply_len = 2;
    

    for(;;) { // rps server loop 
        // wait until client sends message
        Receive(&client_tid, msg, msg_len);

        // handle the request 
        switch(msg[0]) {
            case RPS_SIGNUP: {
                if(rps_server.waiting_player == -1) { // no waiting player
                    rps_server.waiting_player = client_tid;
                    uart_printf(CONSOLE, "[RPS]: Player %d signed up, waiting for partner...\r\n", client_tid);
                    reply[0] = RPS_WAIT_FOR_PARTNER;
                }
                else { // partner up with waiting player
                    rps_server.players[client_tid] = rps_server.waiting_player;
                    rps_server.players[rps_server.waiting_player] = client_tid;
                    rps_server.waiting_player = -1;
                    uart_printf(CONSOLE, "[RPS]: Player %d signed up, partner: %d\r\n", client_tid, rps_server.players[client_tid]);
                    reply[0] = RPS_READY;
                    Reply(client_tid, reply, reply_len);
                    Reply(rps_server.players[client_tid], reply, reply_len);
                }
                break;
            }
            case RPS_PLAY: {
                if(rps_server.players[client_tid] == -2) { // partner quit
                    uart_printf(CONSOLE, "[RPS]: Player %d's partner quit.\r\n", client_tid);
                    rps_server.players[client_tid] = -1;
                    reply[0] = RPS_PARTNER_QUIT;
                    Reply(client_tid, reply, reply_len);
                    break;
                }
                else if(rps_server.players[client_tid] == -1) { // waiting to get a partner
                    uart_printf(CONSOLE, "[RPS]: Player %d does not have a partner.\r\n", client_tid);
                    reply[0] = RPS_WAIT_FOR_PARTNER;
                    break;
                }
                if(rps_server.moves[client_tid] != RPS_NOT_PLAYED) { // player has already played
                    uart_printf(CONSOLE, "[RPS]: Player %d has already played in this round.\r\n", client_tid);
                    reply[0] = RPS_WAIT_FOR_PARTNER;
                    break;
                }  
                switch(msg[1]) {
                    case RPS_ROCK:
                        rps_server.moves[client_tid] = RPS_ROCK;
                        uart_printf(CONSOLE, "[RPS]: Player %d played Rock against player %d!\r\n", client_tid, rps_server.players[client_tid]);
                        break;
                    case RPS_PAPER:
                        rps_server.moves[client_tid] = RPS_PAPER;
                        uart_printf(CONSOLE, "[RPS]: Player %d played Paper against player %d!\r\n", client_tid, rps_server.players[client_tid]);
                        break;
                    case RPS_SCISSORS:
                        rps_server.moves[client_tid] = RPS_SCISSORS;
                        uart_printf(CONSOLE, "[RPS]: Player %d played Scissors against player %d!\r\n", client_tid, rps_server.players[client_tid]);
                        break;
                    default: break;
                }
                if(rps_server.moves[rps_server.players[client_tid]] != RPS_NOT_PLAYED) { // partner has already played, decide winner
                    // get rps round winner
                    int winner = rps_server.getWinner(client_tid, rps_server.players[client_tid]);
                    
                    if(winner == client_tid) {
                        uart_printf(CONSOLE, "[RPS]: \033[32mPlayer %d won this round against player %d!\033[33m\r\n\n", winner, rps_server.players[winner]);
                        reply[0] = RPS_WIN;
                        Reply(client_tid, reply, reply_len);
                        reply[0] = RPS_LOSS;
                        Reply(rps_server.players[client_tid], reply, reply_len);
                    }
                    else if(winner == rps_server.players[client_tid]) {
                        uart_printf(CONSOLE, "[RPS]: \033[32mPlayer %d won this round against player %d!\033[33m\r\n\n", winner, rps_server.players[winner]);
                        reply[0] = RPS_WIN;
                        Reply(rps_server.players[client_tid], reply, reply_len);
                        reply[0] = RPS_LOSS;
                        Reply(client_tid, reply, reply_len);
                    }
                    else {
                        uart_printf(CONSOLE, "[RPS]: \033[32mPlayer %d and player %d Tied!\033[33m\r\n\n", client_tid, rps_server.players[client_tid]);
                        reply[0] = RPS_TIE;
                        Reply(rps_server.players[client_tid], reply, reply_len);
                        Reply(client_tid, reply, reply_len);
                    }
                    // reset moves for next round
                    rps_server.moves[rps_server.players[client_tid]] = RPS_NOT_PLAYED;
                    rps_server.moves[client_tid] = RPS_NOT_PLAYED;
                }
                break;
            }
            case RPS_QUIT: {
                // reset partners and moves
                reply[0] = RPS_QUIT;
                if(rps_server.moves[rps_server.players[client_tid]] != RPS_NOT_PLAYED) {
                    Reply(rps_server.players[client_tid], reply, reply_len);
                }
                rps_server.players[rps_server.players[client_tid]] = -1;
                rps_server.players[client_tid] = -1;
                rps_server.moves[rps_server.players[client_tid]] = RPS_NOT_PLAYED;
                rps_server.moves[client_tid] = RPS_NOT_PLAYED;
                uart_printf(CONSOLE, "[RPS]: \033[31mPlayer %d quit.\033[33m\r\n", client_tid);
                Reply(client_tid, reply, reply_len);
                break;
            }
            default: 
                uart_printf(CONSOLE, "[RPS]: Tid %d not in player list/invalid command\r\n", client_tid);
                break;
        }

    }

    uart_printf(CONSOLE, "Exiting RPS server...\r\n");
    Exit();

}

void rps_client() {

    // find rps server's tid
    volatile int server_tid = -1;
    while(server_tid < 0) server_tid = WhoIs("rps_server");
    
    char msg[2];
    char reply[2];
    volatile int msg_len = 2;
    volatile int reply_len = 2;
    

    for(int i = 0; i < 2; ++i) { // number of rounds of 2 moves each
        // player signup
        msg[0] = RPS_SIGNUP;
        msg[1] = RPS_NOT_PLAYED;

        volatile int retval = -1;
        retval = Send(server_tid, msg, msg_len, reply, reply_len);
        
        // play a random move
        msg[0] = RPS_PLAY;
        int rand_num = rand(0, 2);
        if(rand_num == 0) msg[1] = RPS_ROCK;
        else if(rand_num == 1) msg[1] = RPS_PAPER;
        else if(rand_num == 2) msg[1] = RPS_SCISSORS;
        retval = Send(server_tid, msg, msg_len, reply, reply_len);
        if(retval == RPS_PARTNER_QUIT) goto player_quit;

        // play a random move
        msg[0] = RPS_PLAY;
        rand_num = rand(0, 2);
        if(rand_num == 0) msg[1] = RPS_ROCK;
        else if(rand_num == 2) msg[1] = RPS_PAPER;
        else if(rand_num == 1) msg[1] = RPS_SCISSORS;
        retval = Send(server_tid, msg, msg_len, reply, reply_len);
        if(retval == RPS_PARTNER_QUIT) goto player_quit;

        // quit
        player_quit:
        msg[0] = RPS_QUIT;
        msg[1] = RPS_NOT_PLAYED;
        retval = Send(server_tid, msg, msg_len, reply, reply_len);
    }

    Exit();
}

void rps_main() {
    // make syscall parameters volatile
    volatile int rps_server_tid;

    // create name server
    Create(3, name_server);

    // create rps server
    rps_server_tid = Create(1, rps_server);
    uart_printf(CONSOLE, "Created RPS, server tid: %d, time: %d \r\n", rps_server_tid, get_milli_seconds());
    
    // create rps clients
    Create(1, rps_client);
    Create(0, rps_client);
    Create(2, rps_client);
    Create(1, rps_client);


    Exit();
}

