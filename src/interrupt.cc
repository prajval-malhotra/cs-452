#include "interrupt.h"
#include "constants/codes.h"
#include "uart.h"

// enable rx timeout, rx, tx
#define UART_ENABLE_RX_TX 0b1110000
#define UART_ENABLE_CTS 0b10

/* UART interrupts */

void enable_uart_interrupts(int line) { // set imsc masks to enable rx, tx, rx timeout
    if (line == MARKLIN) {
        UART_REG(line, UART_IMSC) = UART_ENABLE_RX_TX | UART_ENABLE_CTS;
    } else {
	    UART_REG(line, UART_IMSC) = UART_ENABLE_RX_TX;
    }
}

void disable_uart_interrupts(int line) { // set imsc to 0
	UART_REG(line, UART_IMSC) = 0u;
}

void clear_uart_interrupts(int line) { // clear uart interrupts by writing to icr
    if (line == MARKLIN) {
        UART_REG(line, UART_ICR) = UART_ENABLE_RX_TX;
    } else {
        UART_REG(line, UART_ICR) = UART_ENABLE_RX_TX;
    }
}

void clear_uart_cts_interrupt(int line) {
    if (line == MARKLIN) {
        UART_REG(line, UART_ICR) = UART_ENABLE_CTS;
    }
}

unsigned int get_uart_interrupt(int line) { // return type of interrupt stored in mis
	unsigned int uart_mis_value = UART_REG(line, UART_MIS);
	return uart_mis_value;
}

/* GIC */

uint32_t gic_iar(void) {
	return GICC_REG(GIC_GICC_IAR);
}

void gic_eoir(uint32_t value) {
	GICC_REG(GIC_GICC_EOIR) = value;
}

void enable_interrupt_gic(uint32_t m) {
	uint32_t n = m >> 5; // m DIV 32
	uint32_t offset = m & (32 - 1); // m MOD 32
	GICD_REG(0x100 + (4 * n)) = GICD_REG(0x100 + (4 * n)) | (1 << offset);
	return;
}

void set_targets_gic(uint32_t m) {
	uint32_t n = m >> 2; // m DIV 4
	uint32_t byte_offset = m & (4 - 1); // m MOD 4
	uint32_t cpu_core_0 = 1;
	GICD_REG(0x800 + (4 * n)) = GICD_REG(0x800 + (4 * n)) | (cpu_core_0 << (8 * byte_offset));
	return;
}

void init_gic(void) {
	const uint32_t interrupt_ids[3] = {INTERRUPT_TIMER_C1, INTERRUPT_TIMER_C3, INTERRUPT_UART}; // timers and uart
	for (size_t interrupt_id_ind = 0; interrupt_id_ind < 3; ++interrupt_id_ind) {
		set_targets_gic(interrupt_ids[interrupt_id_ind]);
		enable_interrupt_gic(interrupt_ids[interrupt_id_ind]);
	}
    // enable_uart_interrupts();
}
