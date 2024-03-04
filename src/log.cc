#include <stdarg.h>
#include "log.h"
#include "rpi.h"
#include "util.h"

Log DebugLevel = DEBUG;

void print(Log printLevel, const char *fmt, ...) {
    va_list va;
	va_start(va, fmt);
    if (printLevel >= DebugLevel) {
        switch(DebugLevel) {
            case DEBUG:
                uart_format_print(CONSOLE, fmt, va );
                break;
            case WARNING:
                uart_format_print(CONSOLE, fmt, va );
                break;
            case ERROR:
                uart_format_print(CONSOLE, fmt, va );
                break;
            default: break;
        }
    }
	va_end(va);
}
