#include "tasks/task_functions.h"
#include "system_calls.h"

const unsigned int switch_track_b_loop[22] = { 'S', 'S', 'S', 'S', 'C', 'S', 'S', 'C', 'S', 'S', 'S', 'C', 'C', 'C', 'S', 'S', 'C', 'S', 'C', 'S', 'C', 'S' };

struct switch_info {
    int switch_map[256];
    unsigned int reverse_map[22];
    unsigned char switch_state[22];
    size_t num_switches;
    uint32_t last_activated;
};

static void init_switch_info(switch_info* switches) {
    for (size_t i = 0; i < 256; ++i) {
        switches->switch_map[i] = -1;
    }

    switches->switch_map[1] = 0;
    switches->switch_map[2] = 1;
    switches->switch_map[3] = 2;
    switches->switch_map[4] = 3;
    switches->switch_map[5] = 4;
    switches->switch_map[6] = 5;
    switches->switch_map[7] = 6;
    switches->switch_map[8] = 7;
    switches->switch_map[9] = 8;
    switches->switch_map[10] = 9;
    switches->switch_map[11] = 10;
    switches->switch_map[12] = 11;
    switches->switch_map[13] = 12;
    switches->switch_map[14] = 13;
    switches->switch_map[15] = 14;
    switches->switch_map[16] = 15;
    switches->switch_map[17] = 16;
    switches->switch_map[18] = 17;
    switches->switch_map[0x99] = 18;
    switches->switch_map[0x9A] = 19;
    switches->switch_map[0x9B] = 20;
    switches->switch_map[0x9C] = 21;

    switches->reverse_map[0] = 1;
    switches->reverse_map[1] = 2;
    switches->reverse_map[2] = 3;
    switches->reverse_map[3] = 4;
    switches->reverse_map[4] = 5;
    switches->reverse_map[5] = 6;
    switches->reverse_map[6] = 7;
    switches->reverse_map[7] = 8;
    switches->reverse_map[8] = 9;
    switches->reverse_map[9] = 10;
    switches->reverse_map[10] = 11;
    switches->reverse_map[11] = 12;
    switches->reverse_map[12] = 13;
    switches->reverse_map[13] = 14;
    switches->reverse_map[14] = 15;
    switches->reverse_map[15] = 16;
    switches->reverse_map[16] = 17;
    switches->reverse_map[17] = 18;
    switches->reverse_map[18] = 0x99;
    switches->reverse_map[19] = 0x9A;
    switches->reverse_map[20] = 0x9B;
    switches->reverse_map[21] = 0x9C;

    switches->num_switches = 22;

    for (size_t i = 0; i < switches->num_switches; ++i) {
        switches->switch_state[i] = switch_track_b_loop[i];
    }

    switches->last_activated = 0;
}

static constexpr size_t SWITCH_MSG_LENGTH = 23; // 1 byte for switch
static constexpr size_t SWITCH_TURNOUT_DELAY = 20;

void switch_poller() {
    int parent_tid = MyParentTid();
    int clock_server_tid = WhoIs(CLOCK_SERVER_NAME);
    unsigned int target = Time(clock_server_tid);

    char msg[SWITCH_MSG_LENGTH] = {0, 0};
    char reply[SWITCH_MSG_LENGTH] = {0, 0};

    for (;;) {
        target++;
        DelayUntil(clock_server_tid, target);
        Send(parent_tid, msg, SWITCH_MSG_LENGTH, reply, SWITCH_MSG_LENGTH);
    }
}

void switch_server() {
    switch_info switches;
    init_switch_info(&switches);

    RegisterAs(SWITCH_SERVER_NAME);

    int switch_poller_tid = Create(3, switch_poller);
    int marklin_proprietor_tid = WhoIs(MARKLIN_PROPRIETOR_NAME);
    int console_proprietor_tid = WhoIs(CONSOLE_PROPRIETOR_NAME);

    char msg[SWITCH_MSG_LENGTH];
    char reply[SWITCH_MSG_LENGTH];
    msg[0] = MARKLIN_SWITCH;
    reply[0] = 0;

    for (size_t i = 0; i < switches.num_switches; ++i) {
        msg[0] = MARKLIN_SWITCH;
        if (switches.switch_state[i] == 'C') {
            msg[1] = SWITCH_CURVED;
        } else {
            msg[1] = SWITCH_STRAIGHT;
        }

        msg[2] = switches.reverse_map[i];
        Send(marklin_proprietor_tid, msg, 3, reply, 0);
        // Delay(clock_server_tid, SWITCH_TURNOUT_DELAY);
    }

    for(size_t i = 0; i < switches.num_switches; ++i) {
        msg[i+1] = switches.switch_state[i];
    }

    msg[0] = CONSOLE_UPDATE_SWITCHES;
    Send(console_proprietor_tid, msg, 23, reply, 0); // update display

    int tid;

    size_t delay = SWITCH_TURNOUT_DELAY;
    char request;
    unsigned char switch_num;
    bool draw = false;
    for (;;) {
        Receive(&tid, msg, 3); 
    
        switch_num = msg[1];
        request = msg[2];

        if (tid == switch_poller_tid) { // check delay
            msg[0] = SWITCH_OK;
            Reply(tid, msg, 1);

            if (delay > 0) {
                delay--;
                if (delay == 0) {
                    // send turnout command
                    msg[0] = MARKLIN_SWITCH_TURNOUT;
                    Send(marklin_proprietor_tid, msg, 1, reply, 0);
                }
            }
            continue;
        }

        if (switches.switch_map[switch_num] == -1) {
            msg[0] = SWITCH_ERROR;
            Reply(tid, msg, 1);
            continue;
        }

        msg[0] = SWITCH_OK;
        Reply(tid, msg, 1);

        // send to marklin
        switch (request) {
            case 'C': {
                if (switches.switch_state[switches.switch_map[switch_num]] != 'C') {
                    switches.switch_state[switches.switch_map[switch_num]] = 'C';
                    msg[0] = MARKLIN_SWITCH;
                    msg[1] = SWITCH_CURVED;
                    msg[2] = switch_num;
                    Send(marklin_proprietor_tid, msg, 3, reply, 0);
                    draw = true;
                    delay = SWITCH_TURNOUT_DELAY;
                }
                break;
            }
            case 'S': {
                if (switches.switch_state[switches.switch_map[switch_num]] != 'S') {
                    switches.switch_state[switches.switch_map[switch_num]] = 'S';
                    msg[0] = MARKLIN_SWITCH;
                    msg[1] = SWITCH_STRAIGHT;
                    msg[2] = switch_num;
                    Send(marklin_proprietor_tid, msg, 3, reply, 0);
                    draw = true;
                    delay = SWITCH_TURNOUT_DELAY;
                }
                break;
            }
            default:
                break;
        }

        if (draw) {
            // send draw call
            for(size_t i = 0; i < switches.num_switches; ++i) {
                msg[i+1] = switches.switch_state[i];
            }

            msg[0] = CONSOLE_UPDATE_SWITCHES;
            Send(console_proprietor_tid, msg, 23, reply, 0); // update display

            draw = false;
        }
    }
    Exit();
}
