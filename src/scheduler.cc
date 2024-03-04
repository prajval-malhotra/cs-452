#include "scheduler.h"
#include "kernel.h"
#include "rpi.h"
#include "system_timer.h"
#include "tasks/task_functions.h"
#include <stddef.h>

void Scheduler::init_scheduler() {
    current_task = nullptr;
    for (size_t p = 0; p < NUM_PRIORITIES; ++p) {
        priorities[p].head = nullptr;
        priorities[p].tail = nullptr;
    }
}

int Scheduler::add_task(TaskDescriptor *task) {
    if (task->priority >= NUM_PRIORITIES) {
        return -1;
    }

    task->next = nullptr;
    if (priorities[task->priority].tail == nullptr) {
        priorities[task->priority].head = task;
        priorities[task->priority].tail = task;
        return task->priority;
    }

    priorities[task->priority].tail->next = task;
    priorities[task->priority].tail = task;

    return task->priority;
}

int Scheduler::schedule_task() {
    // move current task to its priority
    if (current_task != nullptr) {
        if(current_task->tid == idle_data.idle_task_tid) { // removing idle task
            idle_data.idle_end = TIMER_REG(TIMER_CLO);
        }
        add_task(current_task);
        current_task = nullptr;
    }

    for (size_t p = 0; p < NUM_PRIORITIES; ++p) {
        if (priorities[p].head != nullptr) {
            current_task = priorities[p].head;
            if (priorities[p].tail == priorities[p].head) {
                // only one element in the queue
                priorities[p].tail = nullptr;
            }
            priorities[p].head = priorities[p].head->next;           
            return 0;
        }
    }

    return -1;
}
