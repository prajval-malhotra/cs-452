#ifndef _system_calls_h_
#define _system_calls_h_ 1

#include <stdint.h>

union parameters {
    uint64_t int_arg;
    char *char_ptr_arg;
};

extern "C" int Create(int priority, void (*function)());
extern "C" int MyTid();
extern "C" int MyParentTid();
extern "C" void Yield();
extern "C" void Exit();

/* Message Passing */

extern "C" int Send(int tid, const char *msg, int msglen, char *reply, int rplen);
extern "C" int Receive(int *tid, char *msg, int msglen);
extern "C" int Reply(int tid, const char *reply, int rplen);

/* Name Server */

extern "C" int RegisterAs(const char *name);
extern "C" int WhoIs(const char *name);

/* Interrupt Processing */

extern "C" int AwaitEvent(int eventid);

/* Clock Server */

extern "C" int Time(int tid);
extern "C" int Delay(int tid, int ticks);
extern "C" int DelayUntil(int tid, int ticks);

/* UART */
extern "C" int Getc(int tid, int channel);
extern "C" int Putc(int tid, int channel, unsigned char c);

/* Custom */
extern "C" int Reboot();

#endif
