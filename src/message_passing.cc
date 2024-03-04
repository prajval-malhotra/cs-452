#include "system_calls/message_passing.h"
#include "tasks/task_functions.h"
#include "util.h"
#include "rpi.h"

int SyscallSend(kernel *k_ptr, int tid) {
    // check if tid is valid
    TaskDescriptor *receiver_task = k_ptr->get_task_by_tid(tid);
    if (receiver_task == nullptr) {
        return -1;
    }

    // add sender to receiver's sender queue
    receiver_task->add_sender_task(k_ptr->scheduler->current_task);
    // un-schedule the current task

    k_ptr->scheduler->current_task->state = SEND_BLOCKED;
    k_ptr->scheduler->current_task = nullptr;

    // if receiver is blocked, add it to scheduler and mark it as ready
    if (receiver_task->state == RECEIVE_BLOCKED) {
        TaskDescriptor *sender_task = receiver_task->pop_sender_task();
        // get the tid
        *(receiver_task->receiver_params.tid) = sender_task->tid;

        // copy the memory
        int n = sender_task->sender_params.msg_len;
        if (receiver_task->receiver_params.msg_len < n) {
            n = receiver_task->receiver_params.msg_len;
        }

        if (receiver_task->tid == NAME_SERVER_TID) {
            receiver_task->receiver_params.msg[0] = sender_task->sender_params.name_request;
            receiver_task->receiver_params.msg++; // point to the byte after 
        }
        if (n > 0) {
            memcpy(receiver_task->receiver_params.msg, sender_task->sender_params.msg, n);
        }

        // store return code for Receive
        *((uint64_t *)receiver_task->stack_ptr) = sender_task->sender_params.msg_len;

        receiver_task->state = READY;
        k_ptr->scheduler->add_task(receiver_task);
    }

    return 0;
}

int SyscallReceive(kernel *k_ptr) {
    TaskDescriptor *sender_task = k_ptr->scheduler->current_task->pop_sender_task();
    if (sender_task == nullptr)
    {
        k_ptr->scheduler->current_task->state = RECEIVE_BLOCKED;
        k_ptr->scheduler->current_task = nullptr;
        return -1; // will get handled elsewhere
    }

    int n = k_ptr->scheduler->current_task->receiver_params.msg_len;
    if (sender_task->sender_params.msg_len < n)
    {
        n = sender_task->sender_params.msg_len;
    }

    // get the tid
    *(k_ptr->scheduler->current_task->receiver_params.tid) = sender_task->tid;
    if (k_ptr->scheduler->current_task->tid == NAME_SERVER_TID)
    {
        k_ptr->scheduler->current_task->receiver_params.msg[0] = sender_task->sender_params.name_request;
        k_ptr->scheduler->current_task->receiver_params.msg++; // point to the byte after
    }
    // copy the memory
    if (n > 0) {
        memcpy(k_ptr->scheduler->current_task->receiver_params.msg, sender_task->sender_params.msg, n);
    }
    return sender_task->sender_params.msg_len;
}

int SyscallReply(kernel *k_ptr, int tid,  const char *reply, int reply_len) {
    TaskDescriptor *sender_task = k_ptr->get_task_by_tid(tid);
    if (sender_task == nullptr)
    {
        return -1;
    }

    if (sender_task->state != SEND_BLOCKED) {
        return -2;
    }

    int n = reply_len;
    if (sender_task->sender_params.reply_len < n)
    {
        n = sender_task->sender_params.reply_len;
    }

    // copy the memory
    if (k_ptr->scheduler->current_task->tid == NAME_SERVER_TID) {
        // specially copy if we are returning a tid
        // don't need to specify reply
        if (reply[0] == (char)-1) {
            *((uint64_t *)sender_task->stack_ptr) = -1;
        } else {
            *((uint64_t *)sender_task->stack_ptr) = reply[0];
        }
    } else {
        if (n > 0) {
            memcpy(sender_task->sender_params.reply, reply, n);
        }
        *((uint64_t *)sender_task->stack_ptr) = reply_len;
    }

    // unblock the sender, add back to scheduler
    sender_task->state = READY;
    k_ptr->scheduler->add_task(sender_task);
    return n;
}
