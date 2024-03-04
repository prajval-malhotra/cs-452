#ifndef _TASK_H_
#define _TASK_H_

#include <stdint.h>
#include <stddef.h>

// keep as a multiple of 16, so we never get unaligned
static const unsigned int STACK_SIZE        = 0x1000;

static constexpr size_t TEMP_MSG_SIZE = 5; // 1 byte request plus 4 byte int

typedef enum TaskState {
    READY = 0,
    SEND_BLOCKED,
    RECEIVE_BLOCKED,
    EVENT_BLOCKED,
} TaskState;

class TaskDescriptor {

    struct SenderQueue {
        TaskDescriptor *head;
        TaskDescriptor *tail;
    };

    struct SenderParameters {
        int name_request;

        const char *msg;
        int msg_len;

        char *reply;
        int reply_len;
    };

    struct ReceiverParameters {
        int *tid;
        char *msg;
        int msg_len;
    };

public:
    uint8_t stack[STACK_SIZE];
    uint8_t *stack_ptr;

    int tid;
    int parent_tid;
    int priority;
    TaskState state;

    void (*function)();

    TaskDescriptor *next;

    char temp_msg[TEMP_MSG_SIZE];
    SenderParameters sender_params;
    ReceiverParameters receiver_params;

    SenderQueue sender_queue;
    TaskDescriptor *next_sender;

    int add_sender_task(TaskDescriptor *task);
    TaskDescriptor *pop_sender_task();

    TaskDescriptor *next_awaiter;

    void init_task(int tid, int parent_tid, int priority, void (* function)());
};

#endif // task.h
