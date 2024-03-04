#include "tasks/task_functions.h"
#include "tasks/print_util.h"
#include "system_calls.h"
#include "rpi.h"
#include "util.h"

static const unsigned int MSG_LEN = 30;
static const unsigned int REPLY_LEN = 30;
static const unsigned int PROPRIETOR_MSG_LEN = 30;
static const unsigned int COMMAND_STRING_LEN = 10;


void command_server() {

}

void parse_server() {

    int switch_server_tid = WhoIs(SWITCH_SERVER_NAME);
    int train_server_tid = WhoIs(TRAIN_SERVER_NAME);
    // int uart_server_tid = WhoIs(UART_SERVER_NAME);

    int tid;
    char msg[MSG_LEN];
    int cmd_len = MSG_LEN;
    char reply[REPLY_LEN];
    reply[0] = 0;

    for(;;) {

        if (Receive(&tid, msg, MSG_LEN) < 0) { // wait for next command
            Exit();
        }

        cmd_len = msg[MSG_LEN-1]; // get length of command
        
        char cmd[PROPRIETOR_MSG_LEN]; // args can only be between 0-255 cause char
        char cmd_string[COMMAND_STRING_LEN]; // store command in each iteration

        int idx = 0;
        int cmd_idx = 0;
        
        int  i = 0;
        while(idx < cmd_len) {

            for(int j = 0; j < COMMAND_STRING_LEN; ++j) cmd_string[j] = '\0'; // clear previous command

            int cmd_string_idx = 0;
            while((msg[idx] >= 'A' && msg[idx] <= 'Z') || (msg[idx] >= 'a' && msg[idx] <= 'z') && cmd_string_idx < COMMAND_STRING_LEN) { // get command
                cmd_string[cmd_string_idx] = msg[idx];
                ++cmd_string_idx;
                ++idx;
            }
            
            ++idx; // skip space and go to next valid character

            if(!strncmp(cmd_string, "tr", 2)) {
                cmd[cmd_idx] = CMD_TRAIN;
            }
            else if(!strncmp(cmd_string, "rv", 2)) {
                cmd[cmd_idx] = CMD_REVERSE;
            }
            else if(!strncmp(cmd_string, "sw", 2)) {
                cmd[cmd_idx] = CMD_SWITCH;
            }
            else if(!strncmp(cmd_string, "tid", 3)) {
                cmd[cmd_idx] = CMD_SEND_TO_TID;
            }
            // else if(!strncmp(cmd_string, "start", 5)) {
            //     cmd[cmd_idx] = MARKLIN_START;
            // }
            // else if(!strncmp(cmd_string, "stop", 4)) {
            //     cmd[cmd_idx] = MARKLIN_STOP;
            // }

            ++cmd_idx;

            while((idx < cmd_len) &&  ((msg[idx] >= '0' && msg[idx] <= '9') || (msg[idx] == 'S' || msg[idx] == 'C')) ) { // get arglist
                if (msg[idx] == 'S' || msg[idx] == 'C') {
                    cmd[cmd_idx] = msg[idx];
                    ++idx;
                } 
                else {
                    cmd[cmd_idx] = a2d(msg[idx]);
                    idx++;

                    while (get_num(msg[idx]) != -1 && msg[idx] != ' ' && idx < cmd_len) {
                        cmd[cmd_idx] = (cmd[cmd_idx] * 10) + a2d(msg[idx]);
                        idx++;
                    }
                }

            } // arglist while

        } // outer while

        /*
        // arglist debug prints
        move_cursor(uart_server_tid, 23, 50);
        print_string(uart_server_tid, "[");
        for(int j = 0; j <= cmd_idx; ++j) {
            print_integer(uart_server_tid, cmd[j]);
            print_string(uart_server_tid, ", ");
        }

        print_string(uart_server_tid, "]");
        print_string(uart_server_tid, "len: ");
        print_integer(uart_server_tid, cmd_idx);
        */

        switch(cmd[0]) {
            case CMD_TRAIN: {
                Send(train_server_tid, cmd, PROPRIETOR_MSG_LEN, reply, REPLY_LEN);
                break;
            }
            case CMD_REVERSE: {
                Send(train_server_tid, cmd, PROPRIETOR_MSG_LEN, reply, REPLY_LEN);
                break;
            }
            case CMD_SWITCH: {
                Send(switch_server_tid, cmd, PROPRIETOR_MSG_LEN, reply, REPLY_LEN);
                break;
            }
            case CMD_SEND_TO_TID: { // hardcode receiver tid for now cmd[1]
                cmd[0] = CONSOLE_TEST_PRINT;
                Send(cmd[1], cmd, PROPRIETOR_MSG_LEN, reply, REPLY_LEN);
                break;
            }
            case 'q': {
                Reboot();
            }
            default : {
                break;
            }
        }

        for(int j = 0; j < cmd_len; ++j) msg[j] = '0';
        
        Reply(tid, reply, 0); // reply to user after executing command
    }

    Exit();
}

void user_loop() {

    int uart_server_tid = WhoIs(UART_SERVER_NAME);
    int console_proprietor_tid = WhoIs(CONSOLE_PROPRIETOR_NAME);

    int parse_server_tid = Create(2, parse_server);

    unsigned int msg_index = 0;
    char msg[MSG_LEN];
    char reply[REPLY_LEN];
    reply[0] = 0;
    
    msg[0] = CONSOLE_INIT;
    Send(console_proprietor_tid, msg, PROPRIETOR_MSG_LEN, reply, REPLY_LEN);
    // msg[0] = MARKLIN_START_SENSOR_TASK;
    // Send(marklin_proprietor_tid, msg, PROPRIETOR_MSG_LEN, reply, REPLY_LEN);
    
    for(size_t i = 0; i < MSG_LEN; ++i) msg[i] = '0';

    // Create(1, init_switches); // initialize switches

    for(;;) {
        for (;;) {
            char c = Getc(uart_server_tid, CONSOLE);
            if(c == '\r') {
                
                msg[MSG_LEN-1] = msg_index;
                Send(parse_server_tid, msg, MSG_LEN, reply, REPLY_LEN);

                for(unsigned int i = 0; i < msg_index; ++i) msg[i] = '0'; // clear out parsed command
                msg_index = 0;

                msg[0] = CONSOLE_NEWLINE;
                Send(console_proprietor_tid, msg, PROPRIETOR_MSG_LEN, reply, REPLY_LEN);
                break;
            }
            /*
            else if(c == 'g') {
                msg[0] = MARKLIN_START;
                Send(marklin_proprietor_tid, msg, PROPRIETOR_MSG_LEN, reply, REPLY_LEN);
                break;
            }
            else if(c == 'x') {
                msg[0] = MARKLIN_STOP;
                Send(marklin_proprietor_tid, msg, PROPRIETOR_MSG_LEN, reply, REPLY_LEN);
                break;
            }
            else if(c == 'p') {
                msg[0] = CONSOLE_UPDATE_SWITCHES;
                Send(console_proprietor_tid, msg, PROPRIETOR_MSG_LEN, reply, REPLY_LEN);
                break;
            }
            */
            else { // buffer input for longer commands
                Putc(uart_server_tid, CONSOLE, c);
                msg[msg_index++] = c;
            }

        }
    }


    Exit();
}