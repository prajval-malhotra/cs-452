#ifndef _system_timer_h_
#define _system_timer_h_ 1

#include "constants/mmio_base.h"
#include <stdint.h>

/************************* SYSTEM TIMER *************************/

static char* const TIMER_BASE = (char*)(MMIO_BASE + 0x003000);

// UART register offsets
// data register
static const uint32_t TIMER_CS =   0x00;
// flag register
static const uint32_t TIMER_CLO =   0x04;
// Integer baud rate divisor
static const uint32_t TIMER_CHI = 0x08;
// fractional baud rate divisor
static const uint32_t TIMER_C0 = 0x0C;
// line control register
static const uint32_t TIMER_C1 = 0x10;
// control register
static const uint32_t TIMER_C2 =   0x14;
// control register
static const uint32_t TIMER_C3 =   0x18;

// dereference the address for given register and directly use address 
#define TIMER_REG(offset) (*(volatile uint32_t*)(TIMER_BASE + offset))

inline unsigned int get_time() {
	return (TIMER_REG(TIMER_CLO));
}
inline unsigned int get_minutes() {
	return (TIMER_REG(TIMER_CLO)/1000000)/60;
}
inline unsigned int get_seconds() {
	return (TIMER_REG(TIMER_CLO)/1000000)%60;
}
inline unsigned int get_deci_seconds() {
	return (TIMER_REG(TIMER_CLO)/10000)%100;
}
inline unsigned int get_milli_seconds() {
	return (TIMER_REG(TIMER_CLO)/1000)%1000;
}

void reset_timer(uint32_t timer);
void set_timer(uint32_t timer);

#endif