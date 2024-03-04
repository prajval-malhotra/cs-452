#ifndef _assert_h_
#define _assert_h_ 1

inline void assert(int expression) {
    if (!expression) {
        __asm__ volatile("svc #0x0F"); // replace with actual assertion SVC code
    }
}

#endif
