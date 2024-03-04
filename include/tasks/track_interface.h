#ifndef _track_interface_h_
#define _track_interface_h_ 1

#include "tasks/first_user_task.h"
#include "tasks/task_functions.h"
#include "system_calls.h"
#include "kernel.h"
#include "rpi.h"
#include "util.h"

// unsigned int switch_number[22] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 12, 14, 15, 16, 17, 18, 153, 154, 155, 156 };
const unsigned int switch_configuration[22] = { 33, 33, 34, 33, 34, 33, 33, 34, 34, 34, 34, 34, 33, 33, 33, 34, 34, 34, 33, 33, 33, 33 };
// const unsigned int switch_configuration[22] = { 34, 34, 33, 34, 33, 34, 34, 33, 33, 33, 33, 33, 34, 34, 34, 33, 33, 33, 34, 34, 34, 34 };

enum TrackCommands {
    START = 96,
    STOP = 97,
};

#endif