#include "tasks/task_functions.h"
#include "system_calls.h"
#include "rpi.h"
#include "util.h"

void client_task() {
    int parent_tid = MyParentTid();
    int tid = MyTid();
    const char msg[1] = "";
    char reply[sizeof(int) * 2];
    Send(parent_tid, msg, 0, reply, sizeof(int) * 2);

    int delay_interval, num_delays;
    memcpy(&delay_interval, reply, sizeof(int));
    memcpy(&num_delays, &reply[sizeof(int)], sizeof(int));

    int clock_server_tid = WhoIs(CLOCK_SERVER_NAME);

    int start_ticks = Time(clock_server_tid);
    int recent_tick = 0;
    for (int i = 0; i < num_delays; ++i) {
        recent_tick = Delay(clock_server_tid, delay_interval);
        uart_printf(CONSOLE, "tid: %d, delay interval: %d, number of delays: %d\r\n", tid, delay_interval, i + 1);
    }
    uart_printf(CONSOLE, "total time for task %d: %d\r\n", tid, recent_tick - start_ticks);

    Exit();
}
