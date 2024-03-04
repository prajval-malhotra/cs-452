#ifndef _LOG_H_
#define _LOG_H_

#include "rpi.h"

typedef enum Log {
    OFF = 0,
    DEBUG,
    WARNING,
    ERROR,
} Log;

extern Log DebugLevel;

void print(Log printLevel, const char *fmt, ...);

#endif // _LOG_H_