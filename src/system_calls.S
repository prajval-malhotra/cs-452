.section ".text.system_calls"

// Task Creation

.global Create
Create:
    svc #0x05
    ret

.global MyTid
MyTid:
    svc #0x04
    ret

.global MyParentTid
MyParentTid:
    svc #0x03
    ret

.global Yield
Yield:
    svc #0x02
    ret

.global Exit
Exit:
    svc #0x01
    ret

// Message Passing

.global Send
Send:
    svc #0x11
    ret

.global Receive
Receive:
    svc #0x12
    ret

.global Reply
Reply:
    svc #0x13
    ret

// Name Server

.global RegisterAs
RegisterAs:
    svc #0x21
    ret

.global WhoIs
WhoIs:
    svc #0x22
    ret

// Interrupt Processing

.global AwaitEvent
AwaitEvent:
    svc #0x31
    ret

// Clock Server

.global Time
Time:
    svc #0x41
    ret

.global Delay
Delay:
    svc #0x42
    ret

.global DelayUntil
DelayUntil:
    svc #0x43
    ret

// UART

.global Getc
Getc:
    svc #0x51
    ret

.global Putc
Putc:
    svc #0x52
    ret

// Custom

.global Reboot
Reboot:
    svc #0x00
    ret
