#include "tasks/task_functions.h"
#include "system_calls.h"
#include "rpi.h"
#include "util.h"
#include "kernel.h"

static constexpr int IDLE_TASK_PRINT_PERIOD = 7;

void idle_task() {
    // uart_printf(CONSOLE, "Started idle task\r\n");
    int console_proprietor_tid = WhoIs(CONSOLE_PROPRIETOR_NAME);
    int clock_server_tid = WhoIs(CLOCK_SERVER_NAME);
    char msg[2];
    msg[0] = CONSOLE_UPDATE_IDLE;
    char reply[1];

    int cur_time = Time(clock_server_tid);
    int last_print_time = cur_time;


    for (;;) {
        idle_data.idle_flag = true;
        __asm__ volatile("wfi");
        idle_data.idle_flag = true;
        msg[1] = idle_data.last_idle_percentage;

        cur_time = Time(clock_server_tid);
        if(cur_time - last_print_time > IDLE_TASK_PRINT_PERIOD) {
            Send(console_proprietor_tid, msg, 2, reply, 0);
            last_print_time = cur_time;
        }
    }

    Exit();
}
