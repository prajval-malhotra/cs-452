#ifndef _print_utils_h_
#define _print_utils_h_ 1

#define print_integer(uart_server_tid, number) \
    do { \
        char _buffer[12]; /* Assuming a maximum of 10 digits for integer */ \
        int _num = (number); \
        int _i = 0; \
        \
        if (_num < 0) { \
            Putc(uart_server_tid, CONSOLE, '-'); \
            _num = -_num; \
        } \
        \
        ui2a(_num, 10, _buffer); \
        while (_buffer[_i] != '\0') { \
            Putc(uart_server_tid, CONSOLE, _buffer[_i]); \
            ++_i; \
        } \
    } while (0)

#define print_string(uart_server_tid, escape_code) \
    do { \
        const char *_escape_code = (escape_code); \
        while (*_escape_code) { \
            Putc(uart_server_tid, CONSOLE, *_escape_code); \
            _escape_code++; \
        } \
    } while(0)

#define move_cursor(uart_server_tid, row, col) \
    do { \
        print_string(uart_server_tid, "\033["); \
        \
        char row_ascii[5]; \
        char col_ascii[5]; \
        \
        ui2a(row, 10, row_ascii); \
        ui2a(col, 10, col_ascii); \
        \
        int i = 0; \
        while(row_ascii[i] != '\0') { \
            Putc(uart_server_tid, CONSOLE, row_ascii[i]); \
            ++i; \
        } \
        Putc(uart_server_tid, CONSOLE, ';'); \
        i = 0; \
        while(col_ascii[i] != '\0') { \
            Putc(uart_server_tid, CONSOLE, col_ascii[i]); \
            ++i; \
        } \
        Putc(uart_server_tid, CONSOLE, 'H'); \
    } while(0)



#endif