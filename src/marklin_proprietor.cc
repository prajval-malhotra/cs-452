#include "tasks/task_functions.h"
#include "tasks/print_util.h"
#include "system_calls.h"
#include "uart.h"

static constexpr size_t MARKLIN_MSG_SIZE = 3;
static constexpr size_t MARKLIN_REPLY_SIZE = 3;

static constexpr size_t POLL_MSG_SIZE = 1;
static constexpr size_t POLL_REPLY_SIZE = 10;

void marklin_poller() {
    unsigned int clock_server_tid = WhoIs(CLOCK_SERVER_NAME);
    unsigned int uart_server_tid = WhoIs(UART_SERVER_NAME);
    unsigned int marklin_proprietor_tid = MyParentTid();

    char msg[POLL_MSG_SIZE];
    msg[0] = 0;
    char reply[POLL_REPLY_SIZE];
    reply[0] = 0;

    Putc(uart_server_tid, MARKLIN, 96);
    Delay(clock_server_tid, 15);
    Putc(uart_server_tid, MARKLIN, 0xC0);
    Delay(clock_server_tid, 15);

    for(;;) {
        msg[0] = MARKLIN_OUTPUT_POLL;
        Send(marklin_proprietor_tid, msg, POLL_MSG_SIZE, reply, POLL_REPLY_SIZE);


        int start_ind = 0;
        MarklinRequest command;
        int arg1, arg2;
        command = (MarklinRequest)reply[start_ind++];
        arg1 = reply[start_ind++];
        arg2 = reply[start_ind++];
        
        switch(command) {
            case MARKLIN_START: {
                Putc(uart_server_tid, MARKLIN, 96);
                break;
            }
            case MARKLIN_STOP: {
                Putc(uart_server_tid, MARKLIN, 97);
                break;
            }
            case MARKLIN_TRAIN: {
                Putc(uart_server_tid, MARKLIN, arg2); // speed
                Putc(uart_server_tid, MARKLIN, arg1); // train number
                break;
            }
            case MARKLIN_SWITCH: {
                Putc(uart_server_tid, MARKLIN, arg1); // number
                Putc(uart_server_tid, MARKLIN, arg2); // position
                // Putc(uart_server_tid, MARKLIN, 32);
                break;
            }
            case MARKLIN_SWITCH_TURNOUT: {
                Putc(uart_server_tid, MARKLIN, 32);
                break;
            }
            case MARKLIN_REVERSE: {
                /*
                Putc(uart_server_tid, MARKLIN, 0); // speed
                Putc(uart_server_tid, MARKLIN, arg2); // train number
                Delay(clock_server_tid, 300);
                Putc(uart_server_tid, MARKLIN, 15); // speed
                Putc(uart_server_tid, MARKLIN, arg2); // train number
                Delay(clock_server_tid, 10);
                Putc(uart_server_tid, MARKLIN, arg1); // speed
                Putc(uart_server_tid, MARKLIN, arg2); // train number                
                */
                Putc(uart_server_tid, MARKLIN, 0xF);
                Putc(uart_server_tid, MARKLIN, arg1);
                break;
            }
            case MARKLIN_POLL_SENSORS: {
                Putc(uart_server_tid, MARKLIN, MARKLIN_POLL);
                break;
            }
            default: break;
        }

    }

    Exit();
}

static constexpr size_t COMMAND_BUFFER_SIZE = 30;

void marklin_proprietor() {

    if(RegisterAs(MARKLIN_PROPRIETOR_NAME) != 0) {
        uart_printf(CONSOLE, "could not register uart_server with the name server\r\n");
        Exit();
    }

    int marklin_poller_tid = Create(3, marklin_poller);

    char msg[MARKLIN_MSG_SIZE];
    msg[0] = 0;
    char reply[MARKLIN_REPLY_SIZE];
    reply[0] = 0;

    bool poller_waiting = false;
    bool polling_sensors = false;

    char commands[COMMAND_BUFFER_SIZE][3]; // 30 commands, 3 chars each
    int head = 0;
    int next_free = 0;

    int tid;
    int arg1, arg2;
    size_t start_ind;
    MarklinRequest command;
    for (;;) {
        start_ind = 0;
        Receive(&tid, msg, MARKLIN_MSG_SIZE);

        // contact uart server with command
        command = (MarklinRequest)msg[start_ind++];
        arg1 = msg[start_ind++];
        arg2 = msg[start_ind++];
        if(command != MARKLIN_OUTPUT_POLL && command != MARKLIN_POLL_SENSORS_END) { // skip sensor polls here
            commands[next_free][0] = command;
            commands[next_free][1] = arg1;
            commands[next_free][2] = arg2;

            next_free = (next_free + 1) % COMMAND_BUFFER_SIZE;
        }
        
        switch (command) {
            case MARKLIN_OUTPUT_POLL: {
                poller_waiting = true;
                break;
            }
            case MARKLIN_START: {
                Reply(tid, reply, MARKLIN_REPLY_SIZE);
                break;
            }
            case MARKLIN_STOP: {
                Reply(tid, reply, MARKLIN_REPLY_SIZE);
                break;
            }
            case MARKLIN_TRAIN: {
                Reply(tid, reply, MARKLIN_REPLY_SIZE);
                break;
            }
            case MARKLIN_SWITCH: {
                Reply(tid, reply, MARKLIN_REPLY_SIZE);
                break;
            }
            case MARKLIN_SWITCH_TURNOUT: {
                Reply(tid, reply, MARKLIN_REPLY_SIZE);
                break;
            }
            case MARKLIN_REVERSE: {
                Reply(tid, reply, MARKLIN_REPLY_SIZE);
                break;
            }
            case MARKLIN_POLL_SENSORS: {
                Reply(tid, reply, MARKLIN_REPLY_SIZE);
                break;
            }
            case MARKLIN_POLL_SENSORS_END: {
                Reply(tid, reply, MARKLIN_REPLY_SIZE);
                polling_sensors = false;
                break;
            }
            default:
                break;
        }

        if(poller_waiting && !polling_sensors) {
            if(head != next_free) { // check if command buffer is empty
                reply[0] = commands[head][0];
                reply[1] = commands[head][1];
                reply[2] = commands[head][2];
                
                head = (head + 1) % COMMAND_BUFFER_SIZE;
                poller_waiting = false;

                if (reply[0] == MARKLIN_POLL_SENSORS) {
                    polling_sensors = true;
                }

                Reply(marklin_poller_tid, reply, 3);
            }
        }
    }

    Exit();
}