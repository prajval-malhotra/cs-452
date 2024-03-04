#ifndef _interrupt_h_
#define _interrupt_h_ 1

#include "rpi.h"
#include "constants/mmio_base.h"
#include "util.h"
#include <stdarg.h>

static char* const GIC_BASE = (char*)(MMIO_BASE + 0x1840000); // USE THIS for actual stuff
// static char* const GIC_BASE = (char*)(MMIO_BASE + 0xB000); // WARNING: qemu base address
// static char* const GIC_BASE = (char*)(0x4C0040000); // WARNING: qemu base address
static char* const GIC_GICD_BASE = (char*)(GIC_BASE + 0x1000);
static char* const GIC_GICC_BASE = (char*)(GIC_BASE + 0x2000);

/* GICC register offsets */
static const uint32_t GIC_GICC_CTLR  =   0x000;
static const uint32_t GIC_GICC_IAR   =   0x00C;
static const uint32_t GIC_GICC_EOIR  =   0x010;

#define GICC_REG(reg_offset) (*(volatile uint32_t*)(GIC_GICC_BASE + reg_offset))

/* GIC Distributor register map */
static const uint32_t GIC_GICD_CTLR			=    0x000;
static const uint32_t GIC_GICD_IGROUPR		=    0x080;
static const uint32_t GIC_GICD_ISENABLE 	=    0x100;
static const uint32_t GIC_GICD_ICENABLE 	=    0x180;
static const uint32_t GIC_GICD_IPRIORITY   	=    0x400;
static const uint32_t GIC_GICD_ITARGETS 	=    0x800;
static const uint32_t GIC_GICD_ICFG 		=    0xc00;

#define GICD_REG(reg_offset) (*(volatile uint32_t*)(GIC_GICD_BASE + reg_offset))

/* UART */
void disable_uart_interrupts(int line);
void clear_uart_interrupts(int line);
void clear_uart_cts_interrupt(int line);
void enable_uart_interrupts(int line);
unsigned int get_uart_interrupt(int line);

/* GIC */
void init_gic();
uint32_t gic_iar();
void gic_eoir(uint32_t value);

#endif