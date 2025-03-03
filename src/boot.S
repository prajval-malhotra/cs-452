// https://www.rpi4os.com/part1-bootstrapping/

// ***************************************
// SCTLR_EL1, System Control Register (EL1)
// Architecture Reference Manual Section D17.2.118
// ***************************************
#define SCTLR_RESERVED (3 << 28) | (3 << 22) | (1 << 20) | (1 << 11)
#define USER_MASK_ACCESS (1 << 9)
#define SCTLR_WFE_WFI_ENABLED (1 << 18) | (1 << 16)
#define SCTLR_VALUE_MMU_DISABLED (SCTLR_RESERVED | USER_MASK_ACCESS | SCTLR_WFE_WFI_ENABLED)

// ***************************************
// HCR_EL2, Hypervisor Configuration Register (EL2)
// Architecture Reference Manual Section D17.2.48
// ***************************************
#define HCR_RW (1 << 31)

// ***************************************
// SPSR_EL2, Saved Program Status Register (EL2)
// Architecture Reference Manual Section C5.2.19
// ***************************************
#define SPSR_MASK_ALL (11 << 6)
#define SPSR_EL1       (5 << 0)
#define SPSR_VALUE (SPSR_MASK_ALL | SPSR_EL1)

// ensure the linker puts this at the start of the kernel image
.section ".text.boot"
.global _start
_start:
    // check processor ID is zero (executing on main core), else loop
//    mrs  x0, mpidr_el1
//    and  x0, x0, #3
//    cbnz x0, exit

    // are we already in EL1?
    mrs  x9, CurrentEL
    and  x9, x9, #8
    cbz  x9, el1_entry

    // otherwise, switch to EL1 by fake exception to return from
    ldr x10, =HCR_RW
    msr hcr_el2, x10

    ldr x11, =SPSR_VALUE
    msr spsr_el2, x11

    adr x12, el1_entry
    msr elr_el2, x12

    eret // -> el1_entry

el1_entry:
    // configure processor and mmu
    ldr x13, =SCTLR_VALUE_MMU_DISABLED
    msr sctlr_el1, x13

    // initialize VBAR_EL1
    ldr x3, =exception_vector_table
    msr VBAR_EL1, x3
    
    // clear the bss section
    ldr x4, =__bss_start
    ldr x5, =__bss_end
    b check_bss_bounds

clear_bss_entry:
    // works because bss is 16-byte aligned
    str xzr, [x4]
    // adjust x4 to move up 8 bytes
    add x4, x4, #8

check_bss_bounds:
    // compare x4 to x5 and branch if equal to program startup
    sub x6, x5, x4
    cbz x6, program_startup
    b clear_bss_entry

program_startup:
    // initialize SP
    msr SPSel, #1
    ldr x14, =stackend
    mov sp, x14
    
    // Jump to our main() routine in C/C++
    bl  kmain

    // use watchdog to reboot
    mov x2, 36
    mov x0, 28
    movk  x2, 0xfe10, lsl 16
    movk  x0, 0xfe10, lsl 16
    mov w3, 1
    mov w1, 32
    movk  w3, 0x5a00, lsl 16
    str w3, [x2]
    movk  w1, 0x5a00, lsl 16
    str w1, [x0]


exit:
    // shouldn't be reached
    wfi
    b  exit

// align stack to 16 bytes
.balign 16
stack:
    .rept 0x10000
    .byte 0
    .endr
stackend:

.section ".bss"
.balign 16
