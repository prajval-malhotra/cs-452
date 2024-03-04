#ifndef _task_allocator_h_
#define _task_allocator_h_ 1

#include <stdint.h>
#include <stddef.h>

#include "task.h"

extern const uint8_t *__td_slab_start;
extern const uint8_t *__td_slab_end;

class TaskAllocator {
private:
    union Slab {
        Slab *next;
        TaskDescriptor td;
    };
    Slab *free_list;
    int global_tid;

public:
    void init_task_allocator();
    TaskDescriptor *allocate_task();
    void free_task(TaskDescriptor *task);
};

#endif
