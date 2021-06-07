.global _start
.section .start

_start:
    // zero bss
    ldr x21, =__bss_start__
    ldr x22, =__bss_end__
_bss_loop:
    str xzr, [x21], #8
    cmp x21, x22
    blt _bss_loop
    
    b main
