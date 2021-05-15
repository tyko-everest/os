.global _vector

.section .vector
.balign 2048
_vector:

	// current EL with SP0
.balign 128
	// synch
	b .
.balign 128
	// IRQ
	b .
.balign 128
	// FIQ
	b .
.balign 128
	// SError
	b .

	// current EL with SPx
.balign 128
	// synch
	mrs x0, ESR_EL1
	mov x1, 26
	ror x0, x0, x1
	mov x1, 0b111111
	and x0, x0, x1
	b .
.balign 128
	// IRQ
	b .
.balign 128
	// FIQ
	b .
.balign 128
	// SError
	b .

	// lower EL with AArch64
.balign 128
	// synch
	mrs x0, ESR_EL1
	mov x1, 26
	ror x0, x0, x1
	mov x1, 0b111111
	and x0, x0, x1
	// x0 now contains EC
	// 0b010101 = 0x15 -> SVC from AArch64
	eret
	b .
.balign 128
	// IRQ
	b .
.balign 128
	// FIQ
	b .
.balign 128
	// SError
	b .

	// lower EL with AArch32
.balign 128
	// synch
	b .
.balign 128
	// IRQ
	b .
.balign 128
	// FIQ
	b .
.balign 128
	// SError
	b .
