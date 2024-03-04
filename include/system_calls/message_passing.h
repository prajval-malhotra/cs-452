#ifndef _message_passing_h_
#define _message_passing_h_ 1

#include "kernel.h"

int SyscallSend(kernel *k_ptr, int tid);
int SyscallReceive(kernel *k_ptr);
int SyscallReply(kernel *k_ptr, int tid, const char *reply, int reply_len);

#endif
