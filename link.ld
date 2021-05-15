
SECTIONS {
    __text_start = .;
    .text 0x80000 : {
        KEEP(*(.init))
        *(.text*)
        . = ALIGN(2048);
        *(.vector)

    }
    __text_end = .;
    . = ALIGN(4);
    __rodata_start = .;
    .rodata : {
        *(.rodata*)
    }
    __rodata_end = .;
    . = ALIGN(4);
    __data_start = .;
    .data : {
        *(.data*)
    }
    __data_end = .;
    . = ALIGN(4);
    __bss_start = .;
    .bss : {
        *(.bss*)
    }
    __bss_end = .;
    . = ALIGN(512);
    __files_start = .;
    .zzz : {
        *(.zzz*)
    }
}
