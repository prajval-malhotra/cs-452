#include "rpi.h"
#include "system_timer.h"
#include "util.h"

#include "task.h"
#include "tasks/first_user_task.h"
#include "tasks/task_functions.h"

#include "kernel.h"
#include "interrupt.h"
#include "system_calls.h"
#include "system_calls/message_passing.h"
#include "system_calls/interrupt_processing.h"

extern "C" unsigned int switch_to_task(uint8_t *task_stack_ptr, uint8_t **ptr_to_task_stack_ptr, union parameters *params);
extern "C" void got_interrupt(void);

/* c++ initialization */
static void callConstructors()
{
	// Start and end points of the constructor list,
	// defined by the linker script.
	extern void (*__init_array_start)();
	extern void (*__init_array_end)();

	for (void (**p)() = &__init_array_start; p < &__init_array_end; ++p) {
		(*p)();
	}
}

/* first thing called in mainloop */
static void initialize_io() {
	/* initialize IO buffers */
	uart_printf(CONSOLE, "\033[?25l"); // hide cursor
	uart_printf(CONSOLE, "\033[2J"); // clear screen
	uart_printf(CONSOLE, "\033[33m"); // change colour to yellow
	// uart_printf(CONSOLE, "\x1b[8;%u;%ut", COLS, ROWS); // set display size
}

static void initialize_cache() {

/* cache initialization */
#ifdef BCACHE
        __asm__ volatile(
            "MRS X0, SCTLR_EL1\n"
            "ORR X0, X0, #(1 << 2)\n"
            "ORR X0, X0, #(1 << 12)\n"
            "MSR SCTLR_EL1, X0\n"
        );
#elif ICACHE
        __asm__ volatile(
            "MRS X0, SCTLR_EL1\n"
            "ORR X0, X0, #(1 << 12)\n"
            "MSR SCTLR_EL1, X0\n"
        );
#elif DCACHE
        __asm__ volatile(
            "MRS X0, SCTLR_EL1\n"
            "ORR X0, X0, #(1 << 2)\n"
            "MSR SCTLR_EL1, X0\n"
        );
#endif

}

static void mainloop() {
	initialize_io();
    initialize_cache();
    init_gic(); // comment out to run on qemu

    // kernel structures
    TaskAllocator task_allocator;
    Scheduler scheduler;
    kernel k;

    union parameters params[5] = {0, 0, 0, 0, 0};

    task_allocator.init_task_allocator();
    scheduler.init_scheduler();
    idle_data.init_idle_data();

    k.init_kernel(&task_allocator, &scheduler);
    

// set sender/receiver priority
#ifdef SENDER
    int sender_priority = 1;
    int receiver_priority = 2;
#elif RECEIVER
    int sender_priority = 1;
    int receiver_priority = 2;
#endif

// initialize tasks
#ifdef PERFORMANCE_MEASUREMENT
    TaskDescriptor *send_task = task_allocator.allocate_task();
    send_task->init_task(++k.global_tid, 1, sender_priority, client_send);

    TaskDescriptor *receive_task = task_allocator.allocate_task();
    receive_task->init_task(++k.global_tid, 1, receiver_priority, client_receive);
    
    k.task_list[send_task->tid] = send_task;
    k.task_list[receive_task->tid] = receive_task;
#else 
    TaskDescriptor *first_task = task_allocator.allocate_task();
    first_task->init_task(++k.global_tid, 0, 8, first_user_task);
    
    k.task_list[first_task->tid] = first_task;

    scheduler.add_task(first_task);
#endif

//schedule tasks
#ifdef SENDER
    scheduler.add_task(send_task);
    scheduler.add_task(receive_task);
#elif RECEIVER
    scheduler.add_task(receive_task);
    scheduler.add_task(send_task);
#endif

    int request;
    idle_data.program_start = TIMER_REG(TIMER_CLO);
    for (;;) {
        if (scheduler.schedule_task() != 0) {
            // break;
        }
        
        { // idle and total task time
            idle_data.task_start_time = TIMER_REG(TIMER_CLO);
            if(scheduler.current_task->tid == idle_data.idle_task_tid) {
                idle_data.idle_start = TIMER_REG(TIMER_CLO);
            }
        }

        request = switch_to_task(scheduler.current_task->stack_ptr, &(scheduler.current_task->stack_ptr), params);
        if (request == 0) {
            break; // reboot
        }
        
        { // idle and total task time
            idle_data.idle_end = TIMER_REG(TIMER_CLO);
            idle_data.task_end_time = TIMER_REG(TIMER_CLO);
            idle_data.total_task_time += (idle_data.task_end_time - idle_data.task_start_time);
        }

        process_request(&k, params, request);
    }
}

extern "C" int kmain() {
	// set up GPIO pins for both console and marklin uarts
	callConstructors();
	gpio_init();

	// not strictly necessary, since line 1 is configured during boot
	// but we'll configure the line anyway, so we know what state it is in
	uart_config_and_enable(CONSOLE);
	uart_config_and_enable(MARKLIN);

	mainloop();

	// exit to boot loader
	return 0;
}
