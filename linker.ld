ENTRY(_start)               /* ELF entry symbol - no strictly needed */

SECTIONS {
    . = 0x80000;            /* start text at this location */
    .text : {                /* boot code must come first */
        KEEP(*(.text.boot))
        *(.text)
    }
    .rodata : {
        *(.rodata)
    }
    .data : {
        *(.data)
    }
    .init_array : {
        __init_array_start = .;
        KEEP (*(.init_array*))
        __init_array_end = .;
    }
    .bss : {
        __bss_start = .; 
        *(.bss)
        *(COMMON)
        __bss_end = .;
    }
    
    /* reserve region of memory for task allocator */
    /* align region to 16 byte bound */
    . = ALIGN(0x10);
    __td_slab_start = .;
    . = . + 0x80000;
    __td_slab_end = .;

    __end = .;
}
