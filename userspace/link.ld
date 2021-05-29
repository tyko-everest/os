SECTIONS {
    . = 0;
    .text : {
        *(.text*)
    }
    . = ALIGN(4);
    .rodata : {
        *(.rodata*)
    }
    . = ALIGN(4096);
    .data : {
        *(.data*)
    }
    . = ALIGN(4);
    __bss_start__ = .;
    .bss : {
        *(.bss*)
    }
    __bss_end__ = .;
}