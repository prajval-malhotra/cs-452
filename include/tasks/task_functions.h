#ifndef _task_functions_h_
#define _task_functions_h_ 1

#include <stddef.h>
#include <stdint.h>

enum NameRequest {
    NAME_REGISTER_AS = 1,
    NAME_WHO_IS
};

extern const int CLOCK_SERVER_TID;
static constexpr const char CLOCK_SERVER_NAME[] = "clock_server";
static constexpr const char UART_SERVER_NAME[] = "uart_server";
static constexpr const char MARKLIN_PROPRIETOR_NAME[] = "marklin_proprietor";
static constexpr const char CONSOLE_PROPRIETOR_NAME[] = "console_proprietor";
static constexpr const char PARSE_SERVER_NAME[] = "parse_server";
static constexpr const char SWITCH_SERVER_NAME[] = "switch_server";
static constexpr const char TRAIN_SERVER_NAME[] = "train_server";

enum ClockRequest {
    CLOCK_TICK = 0,
    CLOCK_TIME,
    CLOCK_DELAY,
    CLOCK_DELAY_UNTIL,
};

enum UartRequest {
    UART_GETC,
    UART_PUTC,
    UART_INPUT_REQUEST,
    UART_OUTPUT_REQUEST,
};

enum ParseServerCommands {
    CMD_TRAIN,
    CMD_REVERSE,
    CMD_SWITCH,
    CMD_SEND_TO_TID,
};

enum MarklinRequest {
    MARKLIN_INIT,
    MARKLIN_START,
    MARKLIN_STOP,
    MARKLIN_TRAIN,
    MARKLIN_SWITCH,
    MARKLIN_SWITCH_TURNOUT,
    MARKLIN_REVERSE,
    MARKLIN_POLL_SENSORS,
    MARKLIN_POLL_SENSORS_END,
    MARKLIN_OUTPUT_POLL,
};

enum MarklinCommand {
    MARKLIN_RESET_MODE_ON = 0xC0,
    MARKLIN_POLL = 0x85,
};

enum ConsoleRequest {
    CONSOLE_INIT,
    CONSOLE_UPDATE_SENSORS,
    CONSOLE_UPDATE_SWITCHES,
    CONSOLE_INIT_SWITCHES,
    CONSOLE_UPDATE_TIME,
    CONSOLE_UPDATE_IDLE,
    CONSOLE_NEWLINE,
    CONSOLE_OUTPUT_POLL,
    CONSOLE_TEST_PRINT,
};

enum SwitchRequest {
    SWITCH_CURVED = 34,
    SWITCH_STRAIGHT = 33,
    SWITCH_ERROR,
    SWITCH_OK,
};

enum TrainRequest {
    TRAIN_SPEED,
    TRAIN_REVERSE,
    TRAIN_ERROR,
    TRAIN_OK,
};

void name_server();

void rps_server();
void rps_client_1();
void rps_client_2();
void rps_main();

void client_receive();
void client_send();

void clock_server();
void clock_notifier();
void client_task();

void uart_server();
void uart_output_worker();
void uart_input_worker();

void idle_task();

void marklin_proprietor();
void console_proprietor();
void sensor_notifier();
void switch_server();
void train_server();

void user_loop();

#endif
