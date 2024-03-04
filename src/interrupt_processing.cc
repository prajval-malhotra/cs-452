#include "system_calls/interrupt_processing.h"
#include "constants/codes.h"
#include "util.h"
#include "rpi.h"
#include "system_timer.h"
#include "interrupt.h"

int SyscallAwaitEvent(kernel *k_ptr, int event_id) {
    TaskDescriptor *awaiting_task = k_ptr->scheduler->current_task;

    switch (event_id) {
        case INTERRUPT_TIMER_C1: {
            // check if event has occured since last check
            if (k_ptr->blocked_events.timer_c1 > 0) { // event is waiting
                // remove the wait, let the task continue
                k_ptr->blocked_events.timer_c1 = 0;
            } else { // wait for event, block
                // enable timer for 10ms if it hasn't been already
                if (k_ptr->blocked_events.timer_c1 == -1) {
                    set_timer(TIMER_C1);
                    k_ptr->blocked_events.timer_c1 = 0;
                }

                // check if we already have a waiting task - problem?
                if (k_ptr->blocked_events.timer_c1_wait) {
                    awaiting_task->next_awaiter = k_ptr->blocked_events.timer_c1_wait;
                } else {
                    // cap the list
                    awaiting_task->next_awaiter = nullptr;
                }

                // save task as waiting for next event
                k_ptr->blocked_events.timer_c1_wait = awaiting_task;
                // remove task from ready queue
                awaiting_task->state = EVENT_BLOCKED;
                k_ptr->scheduler->current_task = nullptr;
            }

            break;
        }
        case INTERRUPT_TIMER_C3: {
            // check if event has occured since last check
            if (k_ptr->blocked_events.timer_c3 > 0) { // event is waiting
                k_ptr->blocked_events.timer_c3 = 0;
                k_ptr->blocked_events.timer_c3_wait = nullptr;
            } else { // wait for event, block
                // enable timer for 10ms if it hasn't been already
                if (k_ptr->blocked_events.timer_c3 == -1) {
                    set_timer(TIMER_C3);
                    k_ptr->blocked_events.timer_c3 = 0;
                }

                // check if we already have a waiting task - problem?
                if (k_ptr->blocked_events.timer_c3_wait) {
                    awaiting_task->next_awaiter = k_ptr->blocked_events.timer_c3_wait;
                } else {
                    // cap the list
                    awaiting_task->next_awaiter = nullptr;
                }

                // save task as waiting for next event
                k_ptr->blocked_events.timer_c3_wait = awaiting_task;
                // remove task from ready queue
                awaiting_task->state = EVENT_BLOCKED;
                k_ptr->scheduler->current_task = nullptr;
            }

            break;
        }
        case INTERRUPT_UART_CONSOLE: {
            if (k_ptr->blocked_events.uart_console > 0) { // event is waiting
                // copy interrupt type into task stack
                (*((int *)awaiting_task->stack_ptr)) = k_ptr->blocked_events.uart_console;
                k_ptr->blocked_events.uart_console = 0;
                break;
            }

            // check if there's a waiting task
            if (k_ptr->blocked_events.uart_console_wait) {
                awaiting_task->next_awaiter = k_ptr->blocked_events.uart_console_wait;
            } else {
                awaiting_task->next_awaiter = nullptr;
            }

            k_ptr->blocked_events.uart_console_wait = awaiting_task;
            // remove task from ready queue
            awaiting_task->state = EVENT_BLOCKED;
            k_ptr->scheduler->current_task = nullptr;

            enable_uart_interrupts(CONSOLE);
            break;
        }
        case INTERRUPT_UART_MARKLIN: {
            if (k_ptr->blocked_events.uart > 0) { // event is waiting
                // copy interrupt type into task stack
                (*((int *)awaiting_task->stack_ptr)) = k_ptr->blocked_events.uart_marklin;
                k_ptr->blocked_events.uart_marklin = 0;
                break;
            }

            // check if there's a waiting task
            if (k_ptr->blocked_events.uart_marklin_wait) {
                awaiting_task->next_awaiter = k_ptr->blocked_events.uart_marklin_wait;
            } else {
                awaiting_task->next_awaiter = nullptr;
            }

            k_ptr->blocked_events.uart_marklin_wait = awaiting_task;
            // remove task from ready queue
            awaiting_task->state = EVENT_BLOCKED;
            k_ptr->scheduler->current_task = nullptr;

            enable_uart_interrupts(MARKLIN);
            break;
        }
        default: // invalid event id
            return -1;
    }

    return 0;
}
