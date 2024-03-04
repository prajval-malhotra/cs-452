#include "tasks/task_functions.h"
#include "uart.h"
#include "system_calls.h"

static constexpr size_t POLLING_INTERVAL = 100; // ticks = 1000ms
static constexpr size_t SENSOR_MSG_SIZE = 12;
static constexpr size_t SENSOR_REPLY_SIZE = 1;

struct sensor_info {
    unsigned char map[5][2];
    size_t num_banks;
    size_t bytes_per;
    size_t bytes_received;
    uint32_t time_requested;
    unsigned char recent[10];
    size_t recent_ind;
    size_t recent_items;
};

static void init_sensor_info(sensor_info* sensors) {
    sensors->num_banks = 5;
    sensors->bytes_per = 2;
    for (size_t i = 0; i < sensors->num_banks; ++i) {
        for (size_t j = 0; j < sensors->bytes_per; ++j) {
            sensors->map[i][j] = 0;
        }
    }
    sensors->bytes_received = 10;
    sensors->time_requested = 0;
    for (size_t i = 0; i < 10; ++i) {
        sensors->recent[i] = 0;
    }
    sensors->recent_ind = 0;
    sensors->recent_items = 0;
}

static int process_sensor_byte(sensor_info* sensors, unsigned char byte) {
    if (sensors->bytes_received == 10) {
        return 0;
    }
    size_t bank_ind = sensors->bytes_received / 2;
    size_t byte_ind = sensors->bytes_received % 2;
    (sensors->bytes_received)++;
    sensors->map[bank_ind][byte_ind] = byte;

    // update recency list
    unsigned char bitmask = 0x80;
    int update_to_recent = 0;
    for (size_t k = 0; k < 8; ++k) {
        if (byte & bitmask) {
            unsigned char val = (bank_ind + 1) << 4 | (k + 8 * byte_ind);
            size_t last_ind = sensors->recent_ind;
            if (last_ind == 0) {
                last_ind = 9;
            } else {
                last_ind--;
            }
            if (val != sensors->recent[last_ind]) {
                sensors->recent[sensors->recent_ind] = val;
                update_to_recent = 1;
                sensors->recent_ind++;
                sensors->recent_ind = sensors->recent_ind % 10;

                if (sensors->recent_items < 10) {
                    sensors->recent_items++;
                }
            }
        }
        bitmask = bitmask >> 1;
    }
    if (update_to_recent) {
        return 1;
    }

    return 0;
}

void sensor_notifier() {
    sensor_info sensors;
    init_sensor_info(&sensors);

    int marklin_proprietor_tid = WhoIs(MARKLIN_PROPRIETOR_NAME);
    int console_proprietor_tid = WhoIs(CONSOLE_PROPRIETOR_NAME);
    int clock_server_tid = WhoIs(CLOCK_SERVER_NAME);
    int uart_server_tid = WhoIs(UART_SERVER_NAME);

    char msg[SENSOR_MSG_SIZE];
    msg[0] = MARKLIN_POLL_SENSORS;
    char reply[SENSOR_REPLY_SIZE];
    reply[0] = 0;

    int start_time = Time(clock_server_tid);
    int target_time = start_time;
    int last_print_time = start_time;
    for (;;) {
        int cur_time = Time(clock_server_tid);
        msg[0] = MARKLIN_POLL_SENSORS;
        sensors.bytes_received = 0;
        Send(marklin_proprietor_tid, msg, 1, reply, SENSOR_REPLY_SIZE);        

        // start poll when we get reply        
        for (size_t i = 0; i < 10; ++i) {
            // hangs after entering console command - do we miss this getc and then just wait on data that never arrives?
            process_sensor_byte(&sensors, Getc(uart_server_tid, MARKLIN));
            // Putc(uart_server_tid, MARKLIN, msg[i + 1]);
        }

        // send bytes to display server
        // msg[0] = CONSOLE_UPDATE_SENSORS;
        msg[0] = MARKLIN_POLL_SENSORS_END;
        Send(marklin_proprietor_tid, msg, 1, reply, SENSOR_REPLY_SIZE);

        for (size_t i = 0; i < 10; ++i) {
            msg[i + 3] = sensors.recent[i];
        }
        msg[1] = sensors.recent_ind;
        msg[2] = sensors.recent_items;

        if(cur_time - last_print_time > 10) {
            last_print_time = cur_time;
            msg[0] = CONSOLE_UPDATE_SENSORS;
            Send(console_proprietor_tid, msg, SENSOR_MSG_SIZE, reply, SENSOR_REPLY_SIZE);
        }

        target_time += POLLING_INTERVAL;
        //DelayUntil(clock_server_tid, target_time);
    }

    Exit();
}
