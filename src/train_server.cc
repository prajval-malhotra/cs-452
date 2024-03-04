#include "tasks/task_functions.h"
#include "system_calls.h"
#include "rpi.h"

static constexpr size_t TRAIN_MSG_LENGTH = 3;
static constexpr size_t MAX_TRAINS = 10;

// need to add more logic to support always on headlights, speed range from 1-30
static constexpr size_t HEADLIGHTS = 0;

typedef struct train_info {
    unsigned int speeds[128];
    unsigned int reversing[128];
    unsigned int target_speeds[128];
    size_t num_on_track;
    unsigned int on_track[MAX_TRAINS];
} train_info;

static constexpr size_t TRAIN_REVERSE_DELAY = 400; // 3 seconds

static void init_train_info(train_info* trains) {
    for (size_t i = 0; i < 128; ++i) {
        trains->speeds[i] = 0;
        trains->reversing[i] = 0;
        trains->target_speeds[i] = 0;
    }
    for (size_t i = 0; i < MAX_TRAINS; ++i) {
        trains->on_track[i] = 0;
    }
    trains->num_on_track = 0;
}

static int add_train(train_info* trains, unsigned char train) {
    // add the train to the track if it's never been used before
    for (size_t i = 0; i < trains->num_on_track; ++i) {
        if (trains->on_track[i] == train) {
            return 0;
        }
    }

    if (trains->num_on_track >= MAX_TRAINS) {
        return 1;
    }

    trains->on_track[trains->num_on_track] = train;
    (trains->num_on_track)++;

    return 0;
}

void train_poller() {
    int parent_tid = MyParentTid();
    int clock_server_tid = WhoIs(CLOCK_SERVER_NAME);
    unsigned int target = Time(clock_server_tid);

    char msg[TRAIN_MSG_LENGTH] = {0, 0};
    char reply[TRAIN_MSG_LENGTH] = {0, 0};

    for (;;) {
        target++;
        DelayUntil(clock_server_tid, target);
        Send(parent_tid, msg, TRAIN_MSG_LENGTH, reply, TRAIN_MSG_LENGTH);
    }
}

void train_server() {
    train_info trains;
    init_train_info(&trains);

    RegisterAs(TRAIN_SERVER_NAME);

    int train_poller_tid = Create(3, train_poller);
    int marklin_proprietor_tid = WhoIs(MARKLIN_PROPRIETOR_NAME);

    char msg[TRAIN_MSG_LENGTH];
    char reply[TRAIN_MSG_LENGTH];
    msg[0] = MARKLIN_SWITCH;
    reply[0] = 0;

    int tid;

    MarklinRequest request;
    unsigned char train_num;
    unsigned char train_speed;
    for (;;) {
        Receive(&tid, msg, TRAIN_MSG_LENGTH); 
    
        request = (MarklinRequest)msg[0];
        train_num = msg[1];
        train_speed = msg[2];

        reply[0] = TRAIN_OK;

        if (tid == train_poller_tid) {
            Reply(tid, reply, 1);
            for (size_t i = 0; i < 128; ++i) {
                if (trains.reversing[i] > 0) {
                    trains.reversing[i]--;
                    if (trains.reversing[i] == 0) {
                        trains.speeds[i] = trains.target_speeds[i];
                        msg[0] = MARKLIN_REVERSE;
                        msg[1] = i;
                        Send(marklin_proprietor_tid, msg, TRAIN_MSG_LENGTH, reply, 0);
                        msg[0] = MARKLIN_TRAIN;
                        msg[1] = i;
                        msg[2] = trains.speeds[i] + HEADLIGHTS;
                        Send(marklin_proprietor_tid, msg, TRAIN_MSG_LENGTH, reply, 0);
                    }
                }
            }
            continue;
        }

        if (add_train(&trains, train_num) != 0) {
            reply[0] = TRAIN_ERROR;
        }

        Reply(tid, reply, 1);
        
        if (reply[0] == TRAIN_ERROR) {
            continue;
        }

        // send to marklin
        switch (request) {
            case MARKLIN_TRAIN: {
                msg[2] += HEADLIGHTS; // turn on headlights
                Send(marklin_proprietor_tid, msg, TRAIN_MSG_LENGTH, reply, 0);
                trains.speeds[train_num] = train_speed;
                break;
            }
            case MARKLIN_REVERSE: {
                trains.target_speeds[train_num] = trains.speeds[train_num];
                msg[0] = MARKLIN_TRAIN;
                msg[2] = HEADLIGHTS;
                Send(marklin_proprietor_tid, msg, TRAIN_MSG_LENGTH, reply, 0);
                trains.reversing[train_num] = TRAIN_REVERSE_DELAY;
                break;
            }
            default: 
                break;
        } 
    }
    Exit();
}

