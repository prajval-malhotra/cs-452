#include "kernel.h"
#include "system_calls.h"
#include "system_timer.h"
#include "interrupt.h"
#include "uart.h"
#include "constants/codes.h"
#include "system_calls/message_passing.h"
#include "system_calls/interrupt_processing.h"
#include "tasks/task_functions.h"

void IdleData::init_idle_data() {
    idle_flag = false;
    idle_start = 0;
    idle_end = 0;
    idle_total = 0;
    program_start = 0;
    last_idle_percentage = 0;
    idle_task_tid = -1;

    total_task_time = 0;
    task_start_time = 0;
    task_end_time = 0;
}

class IdleData idle_data;

void IdleData::print_idle_data() {
    idle_total += (idle_end - idle_start);
    int current_idle_percentage = ((idle_total * 100) / total_task_time);
    if(idle_data.last_idle_percentage != current_idle_percentage) { // print only if value changed
        idle_data.last_idle_percentage = current_idle_percentage;
        /*
        uart_printf(CONSOLE, "\033[s");
        // move_cursor(1, 1);
        uart_printf(CONSOLE, "\033[1m\033[K\033[35m");
        uart_printf(CONSOLE, "[Idle time]: %u%%", current_idle_percentage);
        uart_printf(CONSOLE, "\033[33m\033[u\033[0m");
        */
    }
    idle_flag = false;
}

void kernel::init_kernel(TaskAllocator *task_allocator_ptr, Scheduler *scheduler_ptr) {
    global_tid = 0;
    task_allocator = task_allocator_ptr;
    scheduler = scheduler_ptr;
    blocked_events.timer_c1 = -1;
    blocked_events.timer_c1_wait = nullptr;

    for(size_t i =0; i < MAX_TASKS; ++i) {
        task_list[i] = nullptr;
    }
}

static void process_system_call(kernel *k_ptr, union parameters *params, unsigned int request) {
    switch (request) {
        case SYSCALL_EXIT: // Exit
                if(k_ptr->get_task_by_tid(k_ptr->scheduler->current_task->tid)) k_ptr->task_list[k_ptr->scheduler->current_task->tid] = nullptr;
                k_ptr->task_allocator->free_task(k_ptr->scheduler->current_task);
                k_ptr->scheduler->current_task = nullptr;

                break;
        case SYSCALL_YIELD: // Yield
            break; // just a reschedule, nothing to be done

        case SYSCALL_MY_PARENT_TID:  // MyParentTid
            *((uint64_t *)k_ptr->scheduler->current_task->stack_ptr) = k_ptr->scheduler->current_task->parent_tid;
            break;

        case SYSCALL_MY_TID: // MyTid 
            *((uint64_t *)k_ptr->scheduler->current_task->stack_ptr) = k_ptr->scheduler->current_task->tid;
            break;

        case SYSCALL_CREATE: { // Create
            /* get Create()'s arguments from user stack */
            int new_task_tid = 0;
            int new_task_priority = params[0].int_arg;
            void (* new_task_function)() = (void (*)()) params[1].int_arg;

            if (new_task_priority < 0 || new_task_priority >= NUM_PRIORITIES) {
                *((uint64_t *)k_ptr->scheduler->current_task->stack_ptr) = -1;
                break;
            }

            TaskDescriptor *new_task = k_ptr->task_allocator->allocate_task();
            if (new_task == nullptr) {
                *((uint64_t *)k_ptr->scheduler->current_task->stack_ptr) = -2;   
                break;
            }

            if (new_task_function == name_server) {
                new_task_tid = NAME_SERVER_TID;
            } else {
                new_task_tid = ++k_ptr->global_tid;
            }
            
            // set the return value to the task id
            *((uint64_t *)k_ptr->scheduler->current_task->stack_ptr) = new_task_tid;

            // initialize new task with arguments
            new_task->init_task(new_task_tid, k_ptr->scheduler->current_task->tid, new_task_priority, new_task_function);
            k_ptr->task_list[new_task->tid] = new_task;
            k_ptr->scheduler->add_task(new_task);
            break;
        }

        case SYSCALL_SEND: { // Send
            /* get Send()'s arguments */
            int receiver_tid = params[0].int_arg;
            const char *msg = (const char *)params[1].char_ptr_arg;
            int msg_len = (int)params[2].int_arg; 
            char *reply = (char *)params[3].char_ptr_arg; 
            int reply_len = (int)params[4].int_arg;

            // store arguments in a structure
            k_ptr->scheduler->current_task->sender_params.msg = msg;
            k_ptr->scheduler->current_task->sender_params.msg_len = msg_len;
            k_ptr->scheduler->current_task->sender_params.reply = reply;
            k_ptr->scheduler->current_task->sender_params.reply_len = reply_len;

            // get return code from this
            SyscallSend(k_ptr, receiver_tid);

            break;
        }

        case SYSCALL_RECEIVE: { // Receive
            /* get Receive()'s arguments */
            int *tid = (int *)params[0].int_arg;
            char *msg = params[1].char_ptr_arg;
            int msg_len = params[2].int_arg; 

            k_ptr->scheduler->current_task->receiver_params.tid = tid;
            k_ptr->scheduler->current_task->receiver_params.msg = msg;
            k_ptr->scheduler->current_task->receiver_params.msg_len = msg_len;

            // get return code from this
            int retval = SyscallReceive(k_ptr);
            if (retval >= 0) {
                *((uint64_t *)k_ptr->scheduler->current_task->stack_ptr) = retval;
            }

            break;
        }

        case SYSCALL_REPLY: { // Reply
            int tid = (int)params[0].int_arg;
            char *reply = params[1].char_ptr_arg;
            int reply_len = (int)params[2].int_arg;

            *((uint64_t *)k_ptr->scheduler->current_task->stack_ptr) = SyscallReply(k_ptr, tid, reply, reply_len);

            break;
        }

        case SYSCALL_REGISTER_AS: { // RegisterAs
            const char *name = (const char *)params[0].char_ptr_arg;

            k_ptr->scheduler->current_task->sender_params.name_request = NAME_REGISTER_AS;
            k_ptr->scheduler->current_task->sender_params.msg = name;
            k_ptr->scheduler->current_task->sender_params.msg_len = MAX_NAME_LEN; 
            // return/reply from the name server will be stored in caller's stack
            k_ptr->scheduler->current_task->sender_params.reply = (char *)k_ptr->scheduler->current_task->stack_ptr;
            k_ptr->scheduler->current_task->sender_params.reply_len = 1;

            if (SyscallSend(k_ptr, NAME_SERVER_TID) != 0) {
                *((uint64_t *)k_ptr->scheduler->current_task->stack_ptr) = -1;
            }

            break;
        }

        case SYSCALL_WHO_IS: { // WhoIs
            const char *name = (const char *)params[0].char_ptr_arg;

            k_ptr->scheduler->current_task->sender_params.name_request = NAME_WHO_IS;
            k_ptr->scheduler->current_task->sender_params.msg = name;
            k_ptr->scheduler->current_task->sender_params.msg_len = MAX_NAME_LEN;
            // return/reply from the name server will be stored in caller's stack
            k_ptr->scheduler->current_task->sender_params.reply = (char *)k_ptr->scheduler->current_task->stack_ptr;
            k_ptr->scheduler->current_task->sender_params.reply_len = 1;

            if (SyscallSend(k_ptr, NAME_SERVER_TID) != 0) {
                *((uint64_t *)k_ptr->scheduler->current_task->stack_ptr) = -1;
            }

            break;
        }

        case SYSCALL_AWAIT_EVENT: { // AwaitEvent
            int eventId = (int)params[0].int_arg;
            SyscallAwaitEvent(k_ptr, eventId);
            break;
        }

        case SYSCALL_TIME: { // Time
            int tid = (int)params[0].int_arg;
            TaskDescriptor *sender_task = k_ptr->scheduler->current_task;
            sender_task->temp_msg[0] = CLOCK_TIME;

            k_ptr->scheduler->current_task->sender_params.msg = sender_task->temp_msg;
            k_ptr->scheduler->current_task->sender_params.msg_len = 1;
            k_ptr->scheduler->current_task->sender_params.reply = sender_task->temp_msg;
            k_ptr->scheduler->current_task->sender_params.reply_len = 0; // 4 bytes for time
            SyscallSend(k_ptr, tid);
            break;
        }

        case SYSCALL_DELAY: { // Delay
            int tid = (int)params[0].int_arg;
            int ticks = (int)params[1].int_arg;
            TaskDescriptor *sender_task = k_ptr->scheduler->current_task;
            sender_task->temp_msg[0] = CLOCK_DELAY;
            memcpy(&(sender_task->temp_msg[1]), &ticks, 4);
            k_ptr->scheduler->current_task->sender_params.msg = sender_task->temp_msg;
            k_ptr->scheduler->current_task->sender_params.msg_len = 5;
            k_ptr->scheduler->current_task->sender_params.reply = sender_task->temp_msg;
            k_ptr->scheduler->current_task->sender_params.reply_len = 0; // 4 bytes for time
            SyscallSend(k_ptr, tid);
            break;
        }

        case SYSCALL_DELAY_UNTIL: { // DelayUntil
            int tid = (int)params[0].int_arg;
            int ticks = (int)params[1].int_arg;
            TaskDescriptor *sender_task = k_ptr->scheduler->current_task;
            sender_task->temp_msg[0] = CLOCK_DELAY_UNTIL;
            memcpy(&(sender_task->temp_msg[1]), &ticks, 4);
            k_ptr->scheduler->current_task->sender_params.msg = sender_task->temp_msg;
            k_ptr->scheduler->current_task->sender_params.msg_len = 5;
            k_ptr->scheduler->current_task->sender_params.reply = nullptr;
            k_ptr->scheduler->current_task->sender_params.reply_len = 0; // 4 bytes for time
            SyscallSend(k_ptr, tid);
            break;
        }

        case SYSCALL_GETC: { // Getc
            // uart_printf(CONSOLE, "Getc called\r\n");
            int tid = (int)params[0].int_arg;
            int channel = (int)params[1].int_arg;
            TaskDescriptor *sender_task = k_ptr->scheduler->current_task;
            sender_task->temp_msg[0] = UART_GETC;
            sender_task->temp_msg[1] = (char)channel;
            sender_task->sender_params.msg = sender_task->temp_msg;
            sender_task->sender_params.msg_len = 2;
            sender_task->sender_params.reply = nullptr;
            sender_task->sender_params.reply_len = 0;
            SyscallSend(k_ptr, tid);
            break;
        }

        case SYSCALL_PUTC: { // Putc
            // for right now, 
            // uart_printf(CONSOLE, "Putc called\r\n");
            int tid = (int)params[0].int_arg;
            int channel = (int)params[1].int_arg;
            unsigned char ch = (unsigned char)params[2].int_arg;
            TaskDescriptor *sender_task = k_ptr->scheduler->current_task;
            sender_task->temp_msg[0] = UART_PUTC;
            sender_task->temp_msg[1] = (char)channel;
            sender_task->temp_msg[2] = ch;
            k_ptr->scheduler->current_task->sender_params.msg = sender_task->temp_msg;
            k_ptr->scheduler->current_task->sender_params.msg_len = 3;
            k_ptr->scheduler->current_task->sender_params.reply = nullptr;
            k_ptr->scheduler->current_task->sender_params.reply_len = 0;
            SyscallSend(k_ptr, tid);
            break;
        }

        default:
            break;
    }
}

static void process_interrupt(kernel *k_ptr, unsigned int request) {
    switch (request) {
        case INTERRUPT_TIMER_C1: { // handle TIMER_C1 interrupt
            // clear CS register for C1
            reset_timer(TIMER_C1);

            if (k_ptr->blocked_events.timer_c1_wait) { // check and add any waiting events to ready queue
                TaskDescriptor *awaiting_task = k_ptr->blocked_events.timer_c1_wait;
                while (awaiting_task) {
                    awaiting_task->state = READY;
                    k_ptr->scheduler->add_task(awaiting_task);
                    *((uint32_t *)awaiting_task->stack_ptr) = 0u;
                    awaiting_task = awaiting_task->next_awaiter;
                }
                k_ptr->blocked_events.timer_c1_wait = nullptr;
                k_ptr->blocked_events.timer_c1 = 0;
            }
            else { // event is now waiting for task
                k_ptr->blocked_events.timer_c1 = 1;
            }

            gic_eoir(request);
            set_timer(TIMER_C1); // reset timer after we clear interrupt, maybe do in awaitevent
            break;
        }

        case INTERRUPT_TIMER_C3: { // handle TIMER_C3 interrupt
            // clear CS register for C3
            reset_timer(TIMER_C3);

            if (k_ptr->blocked_events.timer_c3_wait) { // check and add any waiting events to ready queue
                TaskDescriptor *awaiting_task = k_ptr->blocked_events.timer_c3_wait;
                while (awaiting_task) {
                    awaiting_task->state = READY;
                    k_ptr->scheduler->add_task(awaiting_task);
                    *((uint32_t *)awaiting_task->stack_ptr) = 0u;
                    awaiting_task = awaiting_task->next_awaiter;
                }
                k_ptr->blocked_events.timer_c3_wait = nullptr;
                k_ptr->blocked_events.timer_c3 = 0;
            }
            else { // event is now waiting for task
                k_ptr->blocked_events.timer_c3 = 1;
            }

            gic_eoir(request);
            set_timer(TIMER_C3);
            break;
        }

        case INTERRUPT_UART: { // handle uart interrupt
            // need to determine which uart it is
            // this code is for the or of all uarts, need to check console and marklin lines
            // then, need to check whether it's a tx or rx interrupt or whatever other interrupt it could be within the line
                // question: do we care about the other interrupts?

            // uart_printf(CONSOLE, "Received uart interrupt\r\n");

            // grab interrupt type, clear interrupts
            unsigned int console_mis_value = get_uart_interrupt(CONSOLE); // get mis value/cause of uart interrupt
            unsigned int marklin_mis_value = get_uart_interrupt(MARKLIN);
            if (console_mis_value > 0) {
                if (k_ptr->blocked_events.uart_console_wait) {
                    TaskDescriptor *awaiting_task = k_ptr->blocked_events.uart_console_wait;
                    while (awaiting_task) {
                        awaiting_task->state = READY;
                        k_ptr->scheduler->add_task(awaiting_task);
                        *((int *)awaiting_task->stack_ptr) = (int)console_mis_value;
                        awaiting_task = awaiting_task->next_awaiter;
                    }
                    k_ptr->blocked_events.uart_console_wait = nullptr;
                    k_ptr->blocked_events.uart_console = 0;
                } else {
                    k_ptr->blocked_events.uart_console = (int)console_mis_value; // store the interrupt type
                }

                disable_uart_interrupts(CONSOLE); // clear imsc mask
                clear_uart_interrupts(CONSOLE); // set icr
            }
            if (marklin_mis_value > 0) {
                if (k_ptr->blocked_events.uart_marklin_wait) {
                    TaskDescriptor *awaiting_task = k_ptr->blocked_events.uart_marklin_wait;
                    while (awaiting_task) {
                        awaiting_task->state = READY;
                        k_ptr->scheduler->add_task(awaiting_task);
                        *((int *)awaiting_task->stack_ptr) = (int)marklin_mis_value;
                        awaiting_task = awaiting_task->next_awaiter;
                    }
                    k_ptr->blocked_events.uart_marklin_wait = nullptr;
                    k_ptr->blocked_events.uart_marklin = 0;
                } else {
                    k_ptr->blocked_events.uart_marklin = (int)marklin_mis_value; // store the interrupt type
                }

                disable_uart_interrupts(MARKLIN);
                clear_uart_interrupts(MARKLIN);
                clear_uart_cts_interrupt(MARKLIN);
            }

            // uart_printf(CONSOLE, "MIS value (kernel): %u\r\n", mis_value);
            /*
            if (mis_value & 0b101 << 4) {
                uart_printf(CONSOLE, "worked!\r\n");
            }
            */

            // shift console mis value down 10 bits
            
            // do we enable uart interrupts here? no, do in awaitevent
            // enable_uart_interrupts();

            // clear interrupt at gic
            gic_eoir(request);

            break;
        }

        default: 
            break;
    }
}

void process_request(kernel *k_ptr, union parameters *params, unsigned int request) {
    if(request == INTERRUPT_SIGNIFIER) {
        if(idle_data.idle_flag) {
            idle_data.print_idle_data();
        }
        request = gic_iar() & 0xFF; // get last 8 bits for interrupt id
        process_interrupt(k_ptr, request);
    } else {
        process_system_call(k_ptr, params, request);
    }
}