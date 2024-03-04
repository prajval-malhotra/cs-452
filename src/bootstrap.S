#define SPSR_MASK_ALL (11 << 6)
#define SPSR_EL1 (0)
// not sure if we should use the mask
#define SPSR_VALUE (SPSR_EL1)

.section ".text.bootstrap"

.global bootstrap_task
// parameters: x0 has the stack pointer, x1 has the function pointer, x2 has pointer to the stack pointer
bootstrap_task:
    // set up user stack pointer
    msr sp_el0, x0 
    
    // change to user stack and reserve space
    msr spsel, #0
    sub sp, sp, #272

    // save sp_el0 in the address of x2
    mov x9, sp
    str x9, [x2]

    // store function pointer and pointer to stack pointer on user stack
    mov x9, SPSR_VALUE
    msr spsr_el1, x9
    stp x1, x9, [sp, #256]
    
    // change back to kernel stack
    msr spsel, #1

    // return to function call
    ret

.global switch_to_task
// arguments: x0 contains user task stack pointer, x1 contains a pointer to the user task stack pointer
switch_to_task:
    // swing user stack pointer FIRST, so we don't lose the value
    msr sp_el0, x0
    
    // save kernel registers
    sub sp, sp, #256
    // these should have user task stack pointer and pointer to user task stack pointer
    stp x0, x1, [sp, #0]
    stp x2, x3, [sp, #16]
    stp x4, x5, [sp, #32]
    stp x6, x7, [sp, #48]
    stp x8, x9, [sp, #64]
    stp x10, x11, [sp, #80]
    stp x12, x13, [sp, #96]
    stp x14, x15, [sp, #112]
    stp x16, x17, [sp, #128]
    stp x18, x19, [sp, #144]
    stp x20, x21, [sp, #160]
    stp x22, x23, [sp, #176]
    stp x24, x25, [sp, #192]
    stp x26, x27, [sp, #208]
    stp x28, x29, [sp, #224]
    // we don't need to store xzr, could get away with just storing spsr_el1 instead
    mrs x9, spsr_el1
    stp x30, x9, [sp, #240]

    // switch to user stack
    msr spsel, #0

    // load user registers
    // get elr and spsr first
    ldp x0, x1, [sp, #256]
    msr elr_el1, x0
    msr spsr_el1, x1

    ldp x0, x1, [sp, #0]
    ldp x2, x3, [sp, #16]
    ldp x4, x5, [sp, #32]
    ldp x6, x7, [sp, #48]
    ldp x8, x9, [sp, #64]
    ldp x10, x11, [sp, #80]
    ldp x12, x13, [sp, #96]
    ldp x14, x15, [sp, #112]
    ldp x16, x17, [sp, #128]
    ldp x18, x19, [sp, #144]
    ldp x20, x21, [sp, #160]
    ldp x22, x23, [sp, #176]
    ldp x24, x25, [sp, #192]
    ldp x26, x27, [sp, #208]
    ldp x28, x29, [sp, #224]
    ldp x30, xzr, [sp, #240]
    add sp, sp, #272

    // return from "exception"
    eret
