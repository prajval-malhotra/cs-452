#ifndef _scheduler_h_
#define _scheduler_h_ 1

#include "task.h"

// 0 is the highest priority, NUM_PRIORITIES - 1 is the lowest
#define NUM_PRIORITIES 10 

class Scheduler {
    struct ReadyQueue {
        TaskDescriptor *head;
        TaskDescriptor *tail;
    };
    ReadyQueue priorities[NUM_PRIORITIES];

public:
    TaskDescriptor *current_task;

    void init_scheduler();
    int add_task(TaskDescriptor *task);
    int schedule_task();
};

#endif
