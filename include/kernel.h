#ifndef _kernel_h_
#define _kernel_h_ 1

#include "task_allocator.h"
#include "scheduler.h"
#include "constants/task_constants.h"

class InterruptEvents {
public:
    // keeps track of how many tasks are waiting on an event
    int timer_c1;
    int timer_c3;

    int uart;

    int uart_marklin;
    int uart_console;

    // holds the tasks waiting for events
    TaskDescriptor *timer_c1_wait;
    TaskDescriptor *timer_c3_wait;

    TaskDescriptor *uart_wait;    

    TaskDescriptor *uart_marklin_wait;
    TaskDescriptor *uart_console_wait;

    InterruptEvents() : timer_c1(-1), timer_c3(-1), uart(-1), uart_marklin(-1), uart_console(-1),
        timer_c1_wait(NULL), timer_c3_wait(NULL), uart_wait(NULL), uart_marklin_wait(NULL), uart_console_wait(NULL) {}
};

class IdleData {
public:
    bool idle_flag;
    int idle_task_tid;
    int last_idle_percentage;
    uint64_t idle_start;
    uint64_t idle_end;
    uint64_t idle_total;
    uint64_t program_start;

    uint64_t total_task_time;
    uint64_t task_start_time;
    uint64_t task_end_time;

    void init_idle_data();
    void print_idle_data();
};

extern class IdleData idle_data;

class kernel {
public:
    int global_tid;
    TaskAllocator *task_allocator;
    Scheduler *scheduler;
    
    InterruptEvents blocked_events;

    TaskDescriptor *task_list[MAX_TASKS];

    TaskDescriptor *get_task_by_tid(int tid) {
        return task_list[tid];
    }

    void init_kernel(TaskAllocator *task_allocator_ptr, Scheduler *scheduler_ptr);
};

void process_request(kernel *k_ptr, union parameters *params, unsigned int request);

#endif
