#include "tasks/task_functions.h"
#include "tasks/print_util.h"
#include "system_calls.h"
#include "uart.h"
#include "util.h"

/* rows and cols manually have to be set in console proprietor on CONSOLE_INIT */
static constexpr size_t ROWS = 40;
static constexpr size_t COLS = 80;

static constexpr size_t POLLING_INTERVAL = 10;

static constexpr size_t CONSOLE_MSG_SIZE = 25;
static constexpr size_t CONSOLE_REPLY_SIZE = 25;
static constexpr size_t CONSOLE_BUFFER_SIZE = 128;

void time_notifier() {
    unsigned int console_proprietor_tid = WhoIs(CONSOLE_PROPRIETOR_NAME);
    // int uart_server_tid = WhoIs(UART_SERVER_NAME);
    int clock_server_tid = WhoIs(CLOCK_SERVER_NAME);

    char msg[1];
    msg[0] = 0;
    char reply[1];
    reply[0] = 0;

    int target_time = Time(clock_server_tid);

    for(;;) {
        msg[0] = CONSOLE_UPDATE_TIME;
        Send(console_proprietor_tid, msg, 1, reply, 1);

        target_time += POLLING_INTERVAL;
        DelayUntil(clock_server_tid, target_time);
    }
}

void console_poller() {
    int uart_server_tid = WhoIs(UART_SERVER_NAME);
    int clock_server_tid = WhoIs(CLOCK_SERVER_NAME);
    int console_proprietor_tid = MyParentTid();

    int row = 1, col = 1;
    char msg[CONSOLE_MSG_SIZE];
    msg[0] = 0;
    char reply[CONSOLE_REPLY_SIZE];
    reply[0] = 0;

    for(;;) {
        msg[0] = CONSOLE_OUTPUT_POLL;
        Send(console_proprietor_tid, msg, CONSOLE_MSG_SIZE, reply, CONSOLE_REPLY_SIZE);
    
        size_t start_ind = 0;
        ConsoleRequest command;
        command = (ConsoleRequest)reply[start_ind++];

        switch(command) {
            case CONSOLE_INIT: {
                print_string(uart_server_tid, "\x1b[8;80;80t"); // set screen size; cols, rows
                move_cursor(uart_server_tid, row, col);
                print_string(uart_server_tid, "\033[K"); // clear line
                Putc(uart_server_tid, CONSOLE, '>');
                break;
            }
            case CONSOLE_UPDATE_SENSORS: {
                print_string(uart_server_tid, "\033[s");
                move_cursor(uart_server_tid, 5, 1);
                print_string(uart_server_tid, "Recent Sensors\r\n");
                print_string(uart_server_tid, "\033[K");
                /*
                for (int i = 1; i <= 10; ++i) {
                    print_integer(uart_server_tid, msg[i]);
                }
                */
                size_t ind = reply[1];
                if (ind == 0) {
                    ind = 9;
                } else {
                    ind--;
                }
                char number_str[10];
                for (int i = 0; i < reply[2]; ++i) {
                    if (reply[ind + 3] == 0) {
                        break;
                    }
                    unsigned char letter = 'A' - 1 + (reply[ind + 3] >> 4);
                    Putc(uart_server_tid, CONSOLE, letter);
                    ui2a((reply[ind + 3] & 0x0F) + 1, 10, number_str);
                    print_string(uart_server_tid, number_str);
                    if (ind == 0) {
                        ind = 9;
                    } else {
                        ind--;
                    }

                    if (i != reply[2] - 1) {
                        print_string(uart_server_tid, ", ");
                    }
                }

                print_string(uart_server_tid, "\033[u");
                break;
            }
            case CONSOLE_UPDATE_SWITCHES: {
                print_string(uart_server_tid, "\033[s");
                move_cursor(uart_server_tid, 2, 1);
                print_string(uart_server_tid, "Switch States\r\n");
                print_string(uart_server_tid, "\033[K");

                for(int i = 1; i <= 22; ++i) {
                    if (i > 18) {
                        print_integer(uart_server_tid, i + 153 - 19);
                    } else {
                        print_integer(uart_server_tid, i);
                    }
                    Putc(uart_server_tid, CONSOLE, ':');
                    Putc(uart_server_tid, CONSOLE, reply[i]);
                    Putc(uart_server_tid, CONSOLE, ' ');
                    if(i == 11) {
                        print_string(uart_server_tid, "\r\n");
                        print_string(uart_server_tid, "\033[K");
                    }
                }

                print_string(uart_server_tid, "\033[u");
                break;
            }
            case CONSOLE_UPDATE_TIME: {
                print_string(uart_server_tid, "\033[s");
                move_cursor(uart_server_tid, ROWS-1, COLS-20);
                print_string(uart_server_tid, "\033[KTime: ");

                unsigned int cur_time = Time(clock_server_tid);
                print_integer(uart_server_tid, (cur_time/100) / 60);
                Putc(uart_server_tid, CONSOLE, ':');
                print_integer(uart_server_tid, (cur_time/100) % 60); // seconds
                Putc(uart_server_tid, CONSOLE, '.');
                print_integer(uart_server_tid, (cur_time/10) % 10); // deciseconds

                print_string(uart_server_tid, "\033[u");

                break;
            }
            case CONSOLE_NEWLINE: {
                print_string(uart_server_tid, "\033[s");

                move_cursor(uart_server_tid, 1, 1);
                print_string(uart_server_tid, "\033[K");
                Putc(uart_server_tid, CONSOLE, '>');

                // print_string(uart_server_tid, "\033[u");

                break;
            }
            case CONSOLE_UPDATE_IDLE: {
                print_string(uart_server_tid, "\033[s");
                move_cursor(uart_server_tid, ROWS-2, COLS-20);
                print_string(uart_server_tid, "\033[KIdle: ");
                print_integer(uart_server_tid, reply[1]);
                Putc(uart_server_tid, CONSOLE, '%');
                print_string(uart_server_tid, "\033[u");
                break;
            }
            case CONSOLE_TEST_PRINT: {
                print_string(uart_server_tid, "\033[s");
                move_cursor(uart_server_tid, 25, 40);
                print_string(uart_server_tid, "console print test");
                print_string(uart_server_tid, "\033[u");
                break;
            }
            default: {
                break;
            }
        }
    }


    Exit();
}


void console_proprietor() {
    if(RegisterAs(CONSOLE_PROPRIETOR_NAME) != 0) {
        uart_printf(CONSOLE, "could not register uart_server with the name server\r\n");
        Exit();
    }

    Create(3, time_notifier);

    int console_poller_tid = Create(2, console_poller);

    char msg[CONSOLE_MSG_SIZE];
    msg[0] = 0;
    char reply[CONSOLE_REPLY_SIZE];
    reply[0] = 0;

    bool poller_waiting = false;

    char buffer[CONSOLE_BUFFER_SIZE];
    int head = 0;
    int next_free = 0;

    int tid;
    size_t start_ind;
    ConsoleRequest command;
    for(;;) {
        start_ind = 0;
        Receive(&tid, msg, CONSOLE_MSG_SIZE);

        command = (ConsoleRequest)msg[start_ind++];

        if(command != CONSOLE_OUTPUT_POLL) {
            for(size_t i = 0; i < CONSOLE_MSG_SIZE; ++i) {
                buffer[next_free] = msg[i];
                next_free = (next_free + 1) % CONSOLE_BUFFER_SIZE;
            }
        }

        switch(command) {
            case CONSOLE_OUTPUT_POLL: {
                poller_waiting = true;
                break;
            }
            case CONSOLE_INIT: {
                Reply(tid, reply, CONSOLE_REPLY_SIZE);
                break;
            }
            case CONSOLE_UPDATE_SENSORS: {
                // do nothing for now
                Reply(tid, reply, CONSOLE_REPLY_SIZE);
                break;
            }
            case CONSOLE_UPDATE_SWITCHES: {
                Reply(tid, reply, CONSOLE_REPLY_SIZE);
                break;
            }
            case CONSOLE_UPDATE_TIME: {
                Reply(tid, reply, CONSOLE_REPLY_SIZE);
                break;
            }
            case CONSOLE_NEWLINE: {
                Reply(tid, reply, CONSOLE_REPLY_SIZE);
                break;
            }
            case CONSOLE_UPDATE_IDLE: {
                Reply(tid, reply, CONSOLE_REPLY_SIZE);
                break;
            }
            case CONSOLE_TEST_PRINT: {
                Reply(tid, reply, CONSOLE_REPLY_SIZE);
                break;
            }
            default: {
                break;
            }
        }

        if(poller_waiting) {
            if(head != next_free) { // check if command buffer is empty
                for(size_t i = 0; i < CONSOLE_REPLY_SIZE; ++i) {
                    reply[i] = buffer[head];
                    head = (head + 1) % CONSOLE_BUFFER_SIZE;
                }
                
                poller_waiting = false;

                Reply(console_poller_tid, reply, CONSOLE_REPLY_SIZE);
            }
        }

    }

    Exit();
}