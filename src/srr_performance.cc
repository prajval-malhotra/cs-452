#ifndef srr_performance_h_
#define srr_performance_h_ 1

#include "system_calls.h"
#include "rpi.h"
#include "system_timer.h"
#include "util.h"

static const unsigned int ROUNDS = 15;
static const unsigned int ITERATIONS = 10000;

#ifdef DMESSAGE_SIZE_4
    static const unsigned int MSG_SIZE = 4;

#elif DMESSAGE_SIZE_64
    static const unsigned int MSG_SIZE = 64;

#elif DMESSAGE_SIZE_256
    static const unsigned int MSG_SIZE = 256;

#else
    static const unsigned int MSG_SIZE = 4;

#endif


void client_send() {
    
    char msg[MSG_SIZE];

    for(unsigned int i = 0; i < MSG_SIZE; ++i) {
        msg[i] = 5;
    }

    char reply[MSG_SIZE];
    int msg_len = MSG_SIZE;
    int reply_len = MSG_SIZE;
    int start_time = 0;
    int end_time = 0;
    int delay_start = 0;
    int delay_end = 0;
    int avg_time = 0;
    int avg_all_rounds = 0;

    for(unsigned int i = 0; i < ITERATIONS; ++i) {
        delay_start = get_time();
        delay_end = get_time();
    }

    uart_printf(CONSOLE, "Clock delay is %d\u00B5s averaged over %u iterations.\r\n", delay_end-delay_start, ITERATIONS);
    
    uart_printf(CONSOLE, "Printing avaerage over %u iterations x %u Rounds.\r\n", ITERATIONS, ROUNDS);
    uart_printf(CONSOLE, "|-----------------------|\r\n");
    uart_printf(CONSOLE, "|Round\t|\tAverage\t|\r\n");
    uart_printf(CONSOLE, "|-----------------------|\r\n");
    
    for(unsigned int i = 0; i < ROUNDS; ++i) {

        start_time = get_time();

        for(unsigned int j = 0; j < ITERATIONS; ++j) {
            Send(2, msg, msg_len, reply, reply_len);   
        }

        end_time = get_time();

        avg_time += (end_time-start_time)/(ITERATIONS);
        avg_all_rounds += avg_time;

        uart_printf(CONSOLE, "|%d\t|\t%d\u00B5s\t|\r\n", i+1, avg_time);
        uart_printf(CONSOLE, "|-----------------------|\r\n");
        avg_time = 0;

    }

    uart_printf(CONSOLE, "Average over %u rounds: %d\u00B5s\r\n", ROUNDS, avg_all_rounds/ROUNDS);

    Exit();
}

void client_receive() {
    
    int client_tid;
    char msg[MSG_SIZE];
    int msg_len = MSG_SIZE;
    char reply[MSG_SIZE];
    int reply_len = MSG_SIZE;
    int start_time = 0;
    int end_time = 0;
    int avg_time = 0;

    for(unsigned int i = 0; i < MSG_SIZE; ++i) {
        reply[i] = 7;
    }

    for(unsigned int j = 0; j < ROUNDS; ++j) {

        start_time = get_time();
        
        for(unsigned int i = 0; i < ITERATIONS; ++i) {
            Receive(&client_tid, msg, msg_len);

            Reply(1, reply, reply_len);
        }

        end_time = get_time();

        avg_time += (end_time-start_time)/(ITERATIONS);

        avg_time = 0;
    }
    Exit();
}

#endif

