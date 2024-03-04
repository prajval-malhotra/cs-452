#include "task.h"
#include "rpi.h"

extern "C" void bootstrap_task(uint8_t *sp, void (*function)(), uint8_t **sp_ptr);

void TaskDescriptor::init_task(int tid, int parent_tid, int priority, void (*function)()) {
    this->function = function;
    this->stack_ptr = &(this->stack[STACK_SIZE]);
    this->priority = priority;
    this->tid = tid;
    this->state = TaskState::READY;
    this->parent_tid = parent_tid;

    this->next = nullptr;

    this->sender_queue.head = nullptr;
    this->sender_queue.tail = nullptr;
    this->next_sender = nullptr;

    this->next_awaiter = nullptr;

    bootstrap_task(this->stack_ptr, this->function, &(this->stack_ptr));
}

int TaskDescriptor::add_sender_task(TaskDescriptor *task) {
    
    task->next_sender = nullptr;

    if (sender_queue.tail == nullptr) {
        sender_queue.head = task;
        sender_queue.tail = task;
        return task->tid;
    }

    sender_queue.tail->next_sender = task;
    sender_queue.tail = task;

    return task->tid;
}

TaskDescriptor *TaskDescriptor::pop_sender_task() {
    if (sender_queue.head == nullptr) {
        return nullptr;
    }

    TaskDescriptor *task = sender_queue.head;

    if (sender_queue.head == sender_queue.tail) {
        sender_queue.head = nullptr;
        sender_queue.tail = nullptr;
    } else {
        sender_queue.head = sender_queue.head->next_sender;
    }

    return task;
}
