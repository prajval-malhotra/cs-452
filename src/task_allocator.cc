#include "task_allocator.h"
#include "assert.h"
#include "log.h"

static const uint8_t *SLAB_START = (uint8_t *)&__td_slab_start;
static const uint8_t *SLAB_END = (uint8_t *)&__td_slab_end;
static const size_t SLAB_SIZE = (size_t)(SLAB_END - SLAB_START); // might need to use uint64_t
static const size_t TASK_DESCRIPTOR_SIZE_16_BYTE_ALIGN = ((sizeof(TaskDescriptor) + 16 - 1) >> 4) << 4;
static const size_t NUM_SLABS = SLAB_SIZE / TASK_DESCRIPTOR_SIZE_16_BYTE_ALIGN;

void TaskAllocator::init_task_allocator() {
    global_tid = 0;

    // create the free list of Slabs
    free_list = nullptr;
    if (NUM_SLABS == 0) {
        return;
    }

    Slab *current_slab = (Slab *)SLAB_START;
    free_list = current_slab;

    for (size_t slab_ind = 0; slab_ind < NUM_SLABS; ++slab_ind) {
        current_slab->next = (Slab *)((uint64_t)current_slab + TASK_DESCRIPTOR_SIZE_16_BYTE_ALIGN);
        current_slab = current_slab->next;
    }
    current_slab->next = nullptr; // terminate the list
}

TaskDescriptor *TaskAllocator::allocate_task() {
    if (free_list == nullptr) {
        return nullptr;
    }

    TaskDescriptor *task = &(free_list->td);
    free_list = free_list->next;
    return task;
}

void TaskAllocator::free_task(TaskDescriptor *task) {
    Slab *free_slab = (Slab *)task;
    free_slab->next = free_list;
    free_list = free_slab;
}
