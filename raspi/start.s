.section .init
.global _start
.global _delay

@ for now start stack at 512 MB
.equ STACK_START,		0x20000000 - 4

.macro imm32 reg, immediate 
	mov \reg, #(\immediate & 0xFF)
	orr \reg, #(\immediate & 0xFF00)
	orr \reg, #(\immediate & 0xFF0000)
	orr \reg, #(\immediate & 0xFF000000)
.endm

_start:
	imm32 sp, STACK_START
	bl main
_infinite_loop:
	b _infinite_loop

_delay:
	@ only one arg actually given, so rest should be safe to overwrite?
	mov r1, #0
	mov r2, #1
_delay_loop:
	sub r0, r2
	cmp r0, r1
	bne _delay_loop
	bx lr
	