.section ".text.exception_vector_table"
.global exception_vector_table
.balign 0x800
exception_vector_table:
curr_el_sp0_sync:
    b .

.balign 0x80
curr_el_sp0_irq:
    b .

.balign 0x80
curr_el_sp0_fiq:
    b .

.balign 0x80
curr_el_sp0_serror:
    b .

.balign 0x80
curr_el_spx_sync:
    b .

.balign 0x80
curr_el_spx_irq:
    b .

.balign 0x80
curr_el_spx_fiq:
    b .

.balign 0x80
curr_el_spx_serror:
    b .

.balign 0x80
lower_el_aarch64_sync:
    b syscall_to_kernel

.balign 0x80
lower_el_aarch64_irq:
    b interrupt_to_kernel

.balign 0x80
lower_el_aarch64_fiq:
    b .

.balign 0x80
lower_el_aarch64_serror:
    b .

.balign 0x80
lower_el_aarch32_sync:
    b .

.balign 0x80
lower_el_aarch32_irq:
    b .

.balign 0x80
lower_el_aarch32_fiq:
    b .

.balign 0x80
lower_el_aarch32_serror:
    b .

interrupt_to_kernel:
    // switch to user stack
    msr spsel, #0

    // reserve stack space on user stack
    sub sp, sp, #272
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
    stp x30, xzr, [sp, #240]

    // reserve info about exception
    mrs x9, elr_el1
    mrs x10, spsr_el1
    stp x9, x10, [sp, #256]

    // switch back to kernel stack
    msr spsel, #1

    // store the user task's stack pointer
    mrs x9, sp_el0

    // pop kernel registers
    // first, get the pointer to the user task's stack pointer in x1
    ldp x0, x1, [sp, #0]
    str x9, [x1]

    // then, get x30 and spsr
    ldp x30, x9, [sp, #240]
    // msr esr_el1, x0 WE DO NOT OVERWRITE ESR_EL1
    msr spsr_el1, x9

    // set x0 to identifier for interrupts
    mov x0, #0x100

    // x2 has a pointer to the syscall parameters
    ldp x2, x3, [sp, #16]

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

    add sp, sp, #256
    // return to the function we called before
    ret

syscall_to_kernel:
    // switch to user stack
    msr spsel, #0

    // reserve stack space on user stack
    sub sp, sp, #272
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
    stp x30, xzr, [sp, #240]

    // now, we can clobber the registers
    mov x12, x0
    mov x13, x1
    mov x14, x2
    mov x15, x3
    mov x16, x4

    // reserve info about exception
    mrs x9, elr_el1
    mrs x10, spsr_el1
    stp x9, x10, [sp, #256]

    // switch back to kernel stack
    msr spsel, #1

    // store the user task's stack pointer
    mrs x9, sp_el0

    // pop kernel registers
    // first, get the pointer to the user task's stack pointer in x1
    ldp x0, x1, [sp, #0]
    str x9, [x1]

    // then, get x30 and spsr
    ldp x30, x9, [sp, #240]
    // msr esr_el1, x0 WE DO NOT OVERWRITE ESR_EL1
    msr spsr_el1, x9

    // move esr into return register
    mrs x0, esr_el1
    and x0, x0, #0x00FFFFFF // mask out unnecessary bits

    // x2 has a pointer to the syscall parameters
    ldp x2, x3, [sp, #16]
    // we put the syscall parameters into x2
    stp x12, x13, [x2]
    stp x14, x15, [x2, #16]
    str x16, [x2, #32]

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

    add sp, sp, #256
    // return to the function we called before
    ret

