#include "tasks/task_functions.h"
#include "system_calls.h"
#include "rpi.h"
#include "util.h"

#include "constants/task_constants.h"
#include "constants/codes.h"

static constexpr size_t CLOCK_MSG_LEN = 1 + 4; // 4 bytes for the integer value
static constexpr size_t CLOCK_TIME_REPLY_LEN = 4; // 4 bytes for time
static constexpr size_t CLOCK_REPLY_LEN = 1;

void clock_server() {
    // uart_printf(CONSOLE, "creating clock server\r\n");

    uint32_t tick = 0;

    // this will break if tick overflows
    uint32_t delays[MAX_TASKS];
    bool delays_map[MAX_TASKS];
    for (size_t i = 0; i < MAX_TASKS; ++i) {
        delays[i] = 0;
        delays_map[i] = false;
    }

    if (RegisterAs(CLOCK_SERVER_NAME) != 0) {
        uart_printf(CONSOLE, "could not register clock_server with the name server\r\n");
        Exit();
    }
    // give the notifier high priority so it can keep time
    int notifier_tid = Create(0, clock_notifier);
    if (notifier_tid < 0) {
        Exit();
    }

    int tid;
    char msg[CLOCK_MSG_LEN];
    char reply[CLOCK_REPLY_LEN];

    ClockRequest request;
    size_t start_ind;
    for (;;) {
        start_ind = 0;
        if (Receive(&tid, msg, CLOCK_MSG_LEN) < 0) {
            Exit();
        }

        request = (ClockRequest)msg[start_ind++];
        switch (request) {
            case CLOCK_TICK: {
                tick++;
                // uart_printf(CONSOLE, "time: %u, tick: %u\r\n", get_time(), tick);
                Reply(tid, msg, CLOCK_REPLY_LEN);

                for (size_t i = 0; i < MAX_TASKS; ++i) {
                    if (delays_map[i] && delays[i] <= tick) {
                        // uart_printf(CONSOLE, "unblocking %d\r\n", i);
                        delays_map[i] = false;
                        delays[i] = 0;
                        Reply(i, reply, tick);
                    }
                }
                break;
            }
            case CLOCK_TIME:
                if (Reply(tid, reply, tick) < 0) {
                    Exit();
                }
                break;

            case CLOCK_DELAY: {
                int delay;
                memcpy(&delay, &msg[start_ind], CLOCK_MSG_LEN - start_ind);
                if (delay < 0) { // negative delay
                    if (Reply(tid, reply, -2) < 0) {
                        Exit();
                    }
                    break;
                }
                delays[tid] = (uint32_t)delay + tick;
                delays_map[tid] = true;
                // uart_printf(CONSOLE, "got delay from %d\r\n", tid);
                break;
            }

            case CLOCK_DELAY_UNTIL: {
                int delay;
                memcpy(&delay, &msg[start_ind], CLOCK_MSG_LEN - start_ind);
                if (delay < 0) { // negative delay
                    if (Reply(tid, reply, -2) < 0) {
                        Exit();
                    }
                    break;
                }
                delays[tid] = (uint32_t)delay;
                delays_map[tid] = true;
                break;
            }

            default: break;
        }
    }

    Exit();
}

static constexpr size_t CLOCK_NOTIFIER_MSG_LEN = 1;
static constexpr const char CLOCK_NOTIFIER_MSG[] = {CLOCK_TICK};

void clock_notifier() {
    // uart_printf(CONSOLE, "created clock notifier\r\n");

    int server_tid = MyParentTid();
    if (server_tid < 0) {
        Exit();
    }

    char reply[CLOCK_NOTIFIER_MSG_LEN];

    for (;;) {
        if (AwaitEvent(INTERRUPT_TIMER_C1) < 0) {
            Exit();
        }

        if (Send(server_tid, CLOCK_NOTIFIER_MSG, CLOCK_NOTIFIER_MSG_LEN, reply, CLOCK_NOTIFIER_MSG_LEN) < 0) {
            Exit();
        }
    }

    uart_printf(CONSOLE, "clock notifier exiting\r\n");
    Exit();
}