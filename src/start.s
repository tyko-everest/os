.section .init
.global _start
.global _delay

_start:
	// stack must be 16-bytes aligned
	ldr x0, =0x20000000
	mov sp, x0

    ldr x1, =__bss_start
    ldr x2, =__bss_end
_bss_loop:
    str xzr, [x1], #8
    cmp x1, x2
    blt _bss_loop

	bl main
_infinite_loop:
	b _infinite_loop

_delay:
	// only one arg actually given, so rest should be safe to overwrite?
	mov x1, #0
	mov x2, #1
_delay_loop:
	sub x0, x0, x2
	cmp x0, x1
	bne _delay_loop
	ret lr
	