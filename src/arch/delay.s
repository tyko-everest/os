.global _delay
.section .text
_delay:
	sub x0, x0, 1
	cmp x0, xzr
	bne _delay
	ret lr
