.section .text

.equ PERIPH_BASE, 		0x3f000000
.equ GPIO_BASE,			PERIPH_BASE + 0x200000
.equ AUX_BASE,			PERIPH_BASE + 0x215000

.equ GPFSEL0,			GPIO_BASE + 0x00
.equ GPFSEL1,			GPIO_BASE + 0x04
.equ GPSET0,			GPIO_BASE + 0x1C
.equ GPCLR0,			GPIO_BASE + 0x28
.equ GPLEV0,			GPIO_BASE + 0x34

.equ AUX_IRQ,			AUX_BASE + 0x00
.equ AUX_ENB, 			AUX_BASE + 0x04
.equ AUX_MU_IO_REG,		AUX_BASE + 0x40
.equ AUX_MU_BAUD_REG,	AUX_BASE + 0x68

.macro imm32 reg, immediate 
	mov \reg, #(\immediate & 0xFF)
	orr \reg, #(\immediate & 0xFF00)
	orr \reg, #(\immediate & 0xFF0000)
	orr \reg, #(\immediate & 0xFF000000)
.endm

@ set GPIO 14 and 15 to ALT5 (mini UART) and GPIO 18 to output
	imm32 r0, (0b001000000010010 << 12)
	imm32 r1, GPFSEL1
	str r0, [r1]
	@ enable mini UART
	mov r0, #1
	imm32 r1, AUX_ENB
	str r0, [r1]
	@ for baud rate 9600, baud register is 3254
	mov r0, #3254
	imm32 r1, AUX_MU_BAUD_REG
	str r0, [r1]

	@ clear pin
	imm32 r6, GPCLR0
	mov r7, #(1 << 18)
	str r7, [r6]

	@ cpu is set to run at 1 Ghz
	mov r0, #0x41
	imm32 r1, AUX_MU_IO_REG
send_char:
	@ transmit the letter A repeatedly
	str r0, [r1]

	@ @ toggle pin on and off
	@ imm32 r6, GPLEV0
	@ ldr r5, [r6]
	@ mov r7, #(1 << 18)
	@ and r5, r7, r5
	@ cmp r5, r7
	@ beq clear_pin
@ set_pin:
@ 	imm32 r6, GPSET0
@ 	str r7, [r6]
@ 	b done_pin
@ clear_pin:
@ 	imm32 r6, GPCLR0
@ 	mov r7, #(1 << 18)
@ 	str r7, [r6]

done_pin:
	mov r2, #1
	imm32 r3, 1000000
	mov r4, #0
stall_loop:
	sub r3, r3, r2
	cmp r3, r4
	beq send_char
	b stall_loop

	@ put this here for safety
	imm32 r6, GPSET0
	mov r7, #(1 << 18)
	str r7, [r6]
	b done_pin
infinity:
	b infinity
