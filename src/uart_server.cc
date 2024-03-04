#include "tasks/task_functions.h"
#include "system_calls.h"
#include "rpi.h"
#include "util.h"
#include "uart.h"

#include "constants/codes.h"
#include "constants/task_constants.h"

static constexpr size_t UART_MSG_LEN = 1 + 4;
static constexpr size_t UART_REPLY_LEN = 1;

static constexpr size_t OUTPUT_BUFFER_SIZE = 128;
static constexpr size_t INPUT_BUFFER_SIZE = 10;

static constexpr size_t NUM_LINES = 2;

void uart_server() {
    // uart_printf(CONSOLE, "creating uart server\r\n");

    if (RegisterAs(UART_SERVER_NAME) != 0) {
        uart_printf(CONSOLE, "could not register uart_server with the name server\r\n");
        Exit();
    }

    int tid;
    char msg[UART_MSG_LEN];
    char reply[UART_REPLY_LEN];   

    int output_worker_tids[] = {Create(0, uart_output_worker), Create(0, uart_output_worker)};
    int input_worker_tids[] = {Create(0, uart_input_worker), Create(0, uart_input_worker)};
    msg[0] = CONSOLE;
    Send(output_worker_tids[0], msg, 1, reply, 0);
    Send(input_worker_tids[0], msg, 1, reply, 0);
    msg[0] = MARKLIN;
    Send(output_worker_tids[1], msg, 1, reply, 0);
    Send(input_worker_tids[1], msg, 1, reply, 0);

    bool output_workers_waiting[] = {false, false};

    char output_buffers[NUM_LINES][OUTPUT_BUFFER_SIZE + 1];
    // size_t output_heads[] = {0, 0};
    size_t output_tails[] = {1, 1};

    char input_buffers[NUM_LINES][INPUT_BUFFER_SIZE];
    size_t input_heads[] = {0, 0};
    size_t input_tails[] = {0, 0};

    int input_queues[NUM_LINES][MAX_TASKS];
    size_t queue_heads[] = {0, 0};
    size_t queue_tails[] = {0, 0};

    UartRequest request;
    int line;
    size_t start_ind;
    for (;;) {
        start_ind = 0;

        int receive_val = Receive(&tid, msg, UART_MSG_LEN);
        if (receive_val < 0) {
            Exit();
        }

        request = (UartRequest)msg[start_ind++];
        line = ((int)msg[start_ind++]) - 1;
        // uart_printf(CONSOLE, "got a request %d, %d!\r\n", request, line);
        switch (request) {
            case UART_GETC: {
                if (input_heads[line] != input_tails[line]) {
                    Reply(tid, reply, input_buffers[line][input_heads[line]]);
                    input_heads[line] = (input_heads[line] + 1) % INPUT_BUFFER_SIZE;
                } else {
                    // queue up the task
                    input_queues[line][queue_tails[line]] = tid;
                    queue_tails[line] = (queue_tails[line] + 1) % MAX_TASKS;
                }
                break;
            }
            case UART_PUTC: {
                // add to the output buffer if we are not about to overflow
                /*
                if (((output_tails[line] + 1) % OUTPUT_BUFFER_SIZE) != output_heads[line]) {
                    output_buffers[line][output_tails[line]] = msg[start_ind];
                    output_tails[line] = (output_tails[line] + 1) % OUTPUT_BUFFER_SIZE;
                }
                else {
                    // uart_printf(CONSOLE, "\r\noutput Overflow!!!\r\n");
                }
                */
                output_buffers[line][output_tails[line]++] = msg[start_ind];
                Reply(tid, reply, 0);
                if (output_workers_waiting[line]) {
                    /*
                    reply[0] = output_buffers[line][output_heads[line]];
                    output_heads[line] = (output_heads[line] + 1) % OUTPUT_BUFFER_SIZE;
                    Reply(output_worker_tids[line], reply, UART_REPLY_LEN);
                    */
                    output_buffers[line][0] = output_tails[line] - 1;
                    Reply(output_worker_tids[line], output_buffers[line], output_tails[line]);
                    output_tails[line] = 1;
                    output_workers_waiting[line] = false;
                }
                break;
            }
            case UART_INPUT_REQUEST: {
                if (((input_tails[line] + 1) % INPUT_BUFFER_SIZE) != input_heads[line]) {
                    input_buffers[line][input_tails[line]] = msg[start_ind];
                    input_tails[line] = (input_tails[line] + 1) % INPUT_BUFFER_SIZE;
                }
                else {
                    // uart_printf(CONSOLE, "\r\nOverflow!!!\r\n");
                }
                Reply(tid, reply, 0);
                if (queue_heads[line] != queue_tails[line]) {
                    Reply(input_queues[line][queue_heads[line]], reply, input_buffers[line][input_heads[line]]);
                    queue_heads[line] = (queue_heads[line] + 1) % MAX_TASKS;
                    input_heads[line] = (input_heads[line] + 1) % INPUT_BUFFER_SIZE;
                }
                break;
            }
            case UART_OUTPUT_REQUEST: {
                /*
                if (output_heads[line] != output_tails[line]) {
                    // send back a character
                    reply[0] = output_buffers[line][output_heads[line]];
                    output_heads[line] = (output_heads[line] + 1) % OUTPUT_BUFFER_SIZE;
                    Reply(tid, reply, UART_REPLY_LEN);
                    output_workers_waiting[line] = false;

                */
                if (output_tails[line] > 1) {
                    // send back everything
                    output_buffers[line][0] = output_tails[line] - 1;
                    Reply(output_worker_tids[line], output_buffers[line], output_tails[line]);
                    output_tails[line] = 1;
                    output_workers_waiting[line] = false;
                } else {
                    output_workers_waiting[line] = true;
                }
                break;
            }
            
            default: {
                uart_printf(CONSOLE, "In default: %s\r\n", msg[3]);
                break;
            }
        }
    }

    Exit();
}

static constexpr size_t UART_NOTIFIER_MSG_LEN = 3;
static constexpr size_t UART_NOTIFIER_REPLY_LEN = OUTPUT_BUFFER_SIZE + 1;

void uart_output_worker() {
    // uart_printf(CONSOLE, "created uart notifier\r\n");

    int server_tid;

    char msg[UART_NOTIFIER_MSG_LEN] = {0, 0, 0};
    char reply[UART_NOTIFIER_REPLY_LEN];

    Receive(&server_tid, msg, 1);
    int line = msg[0];
    Reply(server_tid, reply, 0);

    msg[0] = UART_OUTPUT_REQUEST;
    msg[1] = line;

    unsigned int mask = 0b1 << 5;
#ifdef TRACK
    unsigned int cts_mask = 0b10;
#endif

    // bool has_output = false;
    // char output_char;
    bool did_output = false;

    size_t output_size = 0;
    size_t output_ind = 1;

    unsigned int mis_value;

    for (;;) {
        if (output_ind == output_size + 1) {
            if (Send(server_tid, msg, UART_NOTIFIER_MSG_LEN, reply, OUTPUT_BUFFER_SIZE) < 0) {
                Exit();
            }
            output_size = reply[0];
            // uart_printf(CONSOLE, "output buffer size %u\r\n", output_size);
            output_ind = 1;
        }

        did_output = false;
        if (!(UART_REG(line, UART_FR) & UART_FR_TXFF) && !(output_ind == output_size + 1)) {
                UART_REG(line, UART_DR) = reply[output_ind++];
                did_output = true;
        } else if (!(output_ind == output_size + 1)) {
            if (line == CONSOLE) {
                mis_value = (unsigned int)AwaitEvent(INTERRUPT_UART_CONSOLE);
            } else {
                mis_value = (unsigned int)AwaitEvent(INTERRUPT_UART_MARKLIN);
            }
            if ((mis_value & mask) && !(output_ind == output_size + 1)) {
                UART_REG(line, UART_DR) = reply[output_ind++];
                did_output = true;
            }
        }

#ifdef TRACK
        if (did_output && line == MARKLIN) {
            for (;;) {
                mis_value = (unsigned int)AwaitEvent(INTERRUPT_UART_MARKLIN);
                if ((mis_value & cts_mask) && (UART_REG(line, UART_FR) & UART_FR_CTS)) {
                    break;
                }
            }
        }
#endif
    }

    // uart_printf(CONSOLE, "uart notifier exiting\r\n");
    Exit();
}

void uart_input_worker() {
    int server_tid; 

    char msg[UART_NOTIFIER_MSG_LEN] = {0, 0, 0};
    char reply[UART_NOTIFIER_REPLY_LEN];

    Receive(&server_tid, msg, 1);
    int line = msg[0];
    Reply(server_tid, reply, 0);

    msg[0] = UART_INPUT_REQUEST;
    msg[1] = line;

    unsigned int mask = 0b101 << 4;

    bool has_input = false;
    char input_char;

    unsigned int mis_value;

    for (;;) {
        if (has_input) {
            msg[2] = input_char;
            if (Send(server_tid, msg, UART_NOTIFIER_MSG_LEN, reply, UART_NOTIFIER_REPLY_LEN) < 0) {
                Exit();
            }
            has_input = false;
        }

        if (!(UART_REG(line, UART_FR) & UART_FR_RXFE) && !has_input) {
            input_char = UART_REG(line, UART_DR);
            has_input = true;
        } else if (!has_input) {
            if (line == CONSOLE) {
                mis_value = (unsigned int)AwaitEvent(INTERRUPT_UART_CONSOLE);
            } else {
                mis_value = (unsigned int)AwaitEvent(INTERRUPT_UART_MARKLIN);
            }
            if ((mis_value & mask) && !has_input) {
                input_char = UART_REG(line, UART_DR);
                has_input = true;
            }
        }
    }

    Exit();
}
