
SECTIONS {
    .text 0x8000 : {
        *(.init)
        *(.text*)
    }
    . = ALIGN(4);
    .data : {
        *(.data)
    }
}
