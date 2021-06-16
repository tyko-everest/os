.global _start
.section .start

_start:
    // zero bss
    ldr x21, =__bss_start__
    ldr x22, =__bss_end__
_bss_loop:
    cmp x21, x22
    beq _main
    str xzr, [x21], #8
    b _bss_loop
_main:
    b main

.rept 4000
    nop
.endr
