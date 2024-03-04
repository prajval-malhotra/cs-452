#ifndef _interrupt_processing_h_
#define _interrupt_processing_h_ 1

#include "kernel.h"

int SyscallAwaitEvent(kernel *k_ptr, int event_id);

#endif