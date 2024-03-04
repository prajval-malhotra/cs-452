#include "tasks/task_functions.h"
#include "constants/task_constants.h"
#include "system_calls.h"
#include "rpi.h"
#include "util.h"

const int NAME_SERVER_TID = 0;
const char NAME_SERVER_NAME[] = "name_server";

static constexpr size_t MAX_NAMED_TASKS = 10; // value of char

struct name_pair {
    char name[MAX_NAME_LEN];
    int tid;
};

static void init_name_map(name_pair *name_map) {
    for (size_t ind = 0; ind < MAX_NAMED_TASKS; ++ind) {
        name_map[ind].name[0] = '\0';
        name_map[ind].tid = -1; 
    }

    name_map[0].tid = NAME_SERVER_TID;
    strncpy(name_map[0].name, NAME_SERVER_NAME, MAX_NAME_LEN);
}

void name_server() {
    // uart_printf(CONSOLE, "creating name server\r\n");

    bool found = false;
    size_t free_ind = MAX_NAMED_TASKS;

    size_t map_ind = 0;
    name_pair name_map[MAX_NAMED_TASKS];

    init_name_map(name_map);

    NameRequest request;
    char response[1]; // response code, response value, null terminator

    int tid;
    char name[MAX_NAME_LEN + 1]; // allow for request code 

    // will execute forever
    size_t start_ind;
    for (;;) {
        start_ind = 0;
        if (Receive(&tid, name, MAX_NAME_LEN + 1) < 0) {
            Exit();
        }

        // get code (either register or find) from name[0]
        request = (NameRequest)name[start_ind++];
        
        switch (request) {
            case NAME_WHO_IS: {
                // uart_printf(CONSOLE, "looking for %s...\r\n", name);
                found = false;
                for (map_ind = 0; map_ind < MAX_NAMED_TASKS; ++map_ind) {
                    if (strncmp(name_map[map_ind].name, &name[start_ind], MAX_NAME_LEN) == 0) {
                        found = true;
 
                        // mapped tid into response array
                        // don't need to worry about truncating; we have low number of tasks
                        response[0] = name_map[map_ind].tid;
                        // uart_printf(CONSOLE, "replying: %d\r\n", response[0]);

                        if (Reply(tid, response, 1) < 0) {
                            Exit();
                        }

                        break;
                    }
                }

                if (!found) {
                    // not found
                    response[0] = -1;

                    if (Reply(tid, response, 1) < 0) {
                        Exit();
                    }
                }
                break;
            }
            case NAME_REGISTER_AS: {
                // check if we already have the same tid
                // check if we already have the same name
                found = false;
                free_ind = MAX_NAMED_TASKS;
                for (map_ind = 0; map_ind < MAX_NAMED_TASKS; ++map_ind) {
                    if (strncmp(name_map[map_ind].name, &name[start_ind], MAX_NAME_LEN) == 0) {
                        // need to overwrite that entry
                        name_map[map_ind].tid = tid;
                        free_ind = MAX_NAMED_TASKS;
                        found = true;

                        response[0] = 0;
                        if (Reply(tid, response, 1) < 0) {
                            Exit();
                        }

                        break;
                    }

                    if (name_map[map_ind].tid == tid) {
                        name_map[map_ind].name[0] = '\0';
                        name_map[map_ind].tid = 0;
                    }

                    if (name_map[map_ind].tid == -1) {
                        free_ind = map_ind;
                    }
                }

                // copy the tid and name into the entry
                if (free_ind != MAX_NAMED_TASKS) {
                    strncpy(name_map[free_ind].name, &name[start_ind], MAX_NAME_LEN);     
                    name_map[free_ind].tid = tid;

                    response[0] = 0;
                    if (Reply(tid, response, 1) < 0) {
                        Exit();
                    }
                } else if (!found) {
                    response[0] = -1;
                    if (Reply(tid, response, 1) < 0) {
                        Exit();
                    }
                }

                break;
            }

            default: 
                break;
        }
    }

    Exit();
}

