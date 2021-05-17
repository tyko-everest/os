
__virtual_start = 0xFFFF000000000000;

SECTIONS {
    . = __virtual_start + 0x80000;
    __text_start = .;
    .text : AT(ADDR(.text) - __virtual_start) {
        KEEP(*(.init))
        *(.text*)
        . = ALIGN(2048);
        *(.vector)

    }
    __text_end = .;
    . = ALIGN(4);
    __rodata_start = .;
    .rodata : AT(ADDR(.rodata) - __virtual_start) {
        *(.rodata*)
    }
    __rodata_end = .;
    . = ALIGN(4);
    __data_start = .;
    .data : AT(ADDR(.data) - __virtual_start) {
        *(.data*)
    }
    __data_end = .;
    . = ALIGN(4);
    __bss_start = .;
    .bss : AT(ADDR(.bss) - __virtual_start) {
        *(.bss*)
    }
    __bss_end = .;
    . = ALIGN(512);
    __files_start = .;
    .zzz : AT(ADDR(.zzz) - __virtual_start) {
        *(.zzz*)
    }
}
