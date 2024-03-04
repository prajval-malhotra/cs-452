#include "tasks/first_user_task.h"
#include "tasks/task_functions.h"
#include "tasks/print_util.h"
#include "system_calls.h"
#include "kernel.h"
#include "rpi.h"
#include "util.h"

/*
void timer_interrupt_task() {
    uart_printf(CONSOLE, "Timer Task.\r\n");
    set_timer();

    while(!TIMER_REG(TIMER_CS) && TIMER_REG(TIMER_C1) != 0) {
        // compiler optimizes out loop without print?
        uart_printf(CONSOLE, "Waiting for interrupt...\r\n");
    }

    uart_printf(CONSOLE, "Interrupt round 2\r\n");
    
    set_timer();

    while(!TIMER_REG(TIMER_CS) && TIMER_REG(TIMER_C1) != 0) {
        uart_printf(CONSOLE, "Waiting for interrupt...\r\n");
    }

    uart_printf(CONSOLE, "Exiting Timer Task.\r\n");
}
*/

/*
void waiting_task() {
    uart_printf(CONSOLE, "Waiting Task.\r\n");

    // AwaitEvent retval is not being set right now
    uint32_t start_time = get_time();
    for (unsigned int i = 0; i < 10; ++i) {
        int retval = AwaitEvent(97);
        uart_printf(CONSOLE, "[user task]: AwaitEvent returned: %u\r\n", (uint32_t)retval);
    }
    uart_printf(CONSOLE, "total time: %u\r\n", get_time() - start_time);

    uart_printf(CONSOLE, "Exiting Waiting Task.\r\n");
}
*/

/* void k3_timer_test() {
    uart_printf(CONSOLE, "FirstUserTask.\r\n");

    Create(8, name_server);

    // make clock server
    Create(1, clock_server);
    // make idle task
    idle_data.idle_task_tid = Create(9, idle_task);

    int tid_1 = Create(3, client_task); 
    int tid_2 = Create(4, client_task);
    int tid_3 = Create(5, client_task);
    int tid_4 = Create(6, client_task);
    // int tid_2 = 0, tid_3 = 0, tid_4 = 0;
    size_t num_tasks = 4;

    int tid;
    char msg[1];
    char reply[sizeof(int) * 2];

    int delay_interval; 
    int num_delays;

    for (size_t i = 0; i < num_tasks; ++i) {
        Receive(&tid, msg, 1);
        if (tid == tid_1) {
            delay_interval = 10;
            num_delays = 20;
        } else if (tid == tid_2) {
            delay_interval = 23;
            num_delays = 9;
        } else if (tid == tid_3) {
            delay_interval = 33;
            num_delays = 6;
        } else if (tid == tid_4) {
            delay_interval = 71;
            num_delays = 3;
        }
        memcpy(reply, &delay_interval, sizeof(int));
        memcpy(&reply[sizeof(int)], &num_delays, sizeof(int));
        Reply(tid, reply, sizeof(int) * 2);
    }

    uart_printf(CONSOLE, "FirstUserTask: exiting\r\n");

    Exit();
}

*/

bool idle_flag = false;

void first_user_task() {
    // uart_printf(CONSOLE, "FirstUserTask.\r\n");

    Create(8, name_server);

    // make clock server
    Create(1, clock_server);
    Create(1, uart_server);
    // make idle task
    idle_data.idle_task_tid = Create(9, idle_task);

    // uart_printf(CONSOLE, "my tid is %d, uart server tid is %d\r\n", MyTid(), uart_server_tid);
    Create(4, marklin_proprietor);
    Create(4, console_proprietor);
    Create(3, user_loop);
    Create(5, sensor_notifier);
    Create(5, switch_server);
    Create(5, train_server);

    /*
    print_string(uart_tid, "console proprietor tid: ");
    print_integer(uart_tid, console_proprietor_tid);
    */

    /*
    char c = 'a';
    Putc(uart_server_tid, CONSOLE, c);
    */

    /*
    const char hello[] = "Hello, World!!!";
    for (size_t i = 0; i < 15; ++i) {
        Putc(uart_server_tid, CONSOLE, hello[i]);
        Putc(uart_server_tid, MARKLIN, hello[i]);
    }

    Create(3, user_loop);

    // for (;;) {
    //     char c = Getc(uart_server_tid, CONSOLE);
    //     Putc(uart_server_tid, CONSOLE, c);
    //     Putc(uart_server_tid, MARKLIN, c);
    // }
    */


    // uart_printf(CONSOLE, "FirstUserTask: exiting\r\n");

    Exit();
}
