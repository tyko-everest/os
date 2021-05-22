.global _vector

.section .vector
.balign 2048
_vector:

	// current EL with SP0
.balign 128
	// synch
	stp x0, x1, [sp, -16]!
	mov x0, 0
	b _vector_common
.balign 128
	// IRQ
	stp x0, x1, [sp, -16]!
	mov x0, 1
	b _vector_common
.balign 128
	// FIQ
	stp x0, x1, [sp, -16]!
	mov x0, 2
	b _vector_common
.balign 128
	// SError
	stp x0, x1, [sp, -16]!
	mov x0, 3
	b _vector_common

	// current EL with SPx
.balign 128
	// synch
	stp x0, x1, [sp, -16]!
	mov x0, 4
	b _vector_common
.balign 128
	// IRQ
	stp x0, x1, [sp, -16]!
	mov x0, 5
	b _vector_common
.balign 128
	// FIQ
	stp x0, x1, [sp, -16]!
	mov x0, 6
	b _vector_common
.balign 128
	// SError
	stp x0, x1, [sp, -16]!
	mov x0, 7
	b _vector_common

	// lower EL with AArch64
.balign 128
	// synch
	stp x0, x1, [sp, -16]!
	mov x0, 8
	b _vector_common
.balign 128
	// IRQ
	stp x0, x1, [sp, -16]!
	mov x0, 9
	b _vector_common
.balign 128
	// FIQ
	stp x0, x1, [sp, -16]!
	mov x0, 10
	b _vector_common
.balign 128
	// SError
	stp x0, x1, [sp, -16]!
	mov x0, 11
	b _vector_common

	// lower EL with AArch32
.balign 128
	// synch
	stp x0, x1, [sp, -16]!
	mov x0, 12
	b _vector_common
.balign 128
	// IRQ
	stp x0, x1, [sp, -16]!
	mov x0, 13
	b _vector_common
.balign 128
	// FIQ
	stp x0, x1, [sp, -16]!
	mov x0, 14
	b _vector_common
.balign 128
	// SError
	stp x0, x1, [sp, -16]!
	mov x0, 15
	b _vector_common

_vector_common:
	stp x2, x3, [sp, -16]!
	stp x4, x5, [sp, -16]!
	stp x6, x7, [sp, -16]!
	stp x8, x9, [sp, -16]!
	stp x10, x11, [sp, -16]!
	stp x12, x13, [sp, -16]!
	stp x14, x15, [sp, -16]!
	stp x16, x17, [sp, -16]!
	stp x18, x19, [sp, -16]!
	stp x20, x21, [sp, -16]!
	stp x22, x23, [sp, -16]!
	stp x24, x25, [sp, -16]!
	stp x26, x27, [sp, -16]!
	stp x28, x29, [sp, -16]!
	stp x30, xzr, [sp, -16]!

	stp q0, q1, [sp, -32]!
	stp q2, q3, [sp, -32]!
	stp q4, q5, [sp, -32]!
	stp q6, q7, [sp, -32]!
	stp q8, q9, [sp, -32]!
	stp q10, q11, [sp, -32]!
	stp q12, q13, [sp, -32]!
	stp q14, q15, [sp, -32]!
	stp q16, q17, [sp, -32]!
	stp q18, q19, [sp, -32]!
	stp q20, q21, [sp, -32]!
	stp q22, q23, [sp, -32]!
	stp q24, q25, [sp, -32]!
	stp q26, q27, [sp, -32]!
	stp q28, q29, [sp, -32]!
	stp q30, q31, [sp, -32]!

	mrs x1, ESR_EL1
	
	bl interrupt_handler
	
	ldp q30, q31, [sp], 32
	ldp q28, q29, [sp], 32
	ldp q26, q27, [sp], 32
	ldp q24, q25, [sp], 32
	ldp q22, q23, [sp], 32
	ldp q20, q21, [sp], 32
	ldp q18, q19, [sp], 32
	ldp q16, q17, [sp], 32
	ldp q14, q15, [sp], 32
	ldp q12, q13, [sp], 32
	ldp q10, q11, [sp], 32
	ldp q8, q9, [sp], 32
	ldp q6, q7, [sp], 32
	ldp q4, q5, [sp], 32
	ldp q2, q3, [sp], 32
	ldp q0, q1, [sp], 32

	ldp x30, xzr, [sp], 16
	ldp x28, x29, [sp], 16
	ldp x26, x27, [sp], 16
	ldp x24, x25, [sp], 16
	ldp x22, x23, [sp], 16
	ldp x20, x21, [sp], 16
	ldp x18, x19, [sp], 16
	ldp x16, x17, [sp], 16
	ldp x14, x15, [sp], 16
	ldp x12, x13, [sp], 16
	ldp x10, x11, [sp], 16
	ldp x8, x9, [sp], 16
	ldp x6, x7, [sp], 16
	ldp x4, x5, [sp], 16
	ldp x2, x3, [sp], 16
	ldp x0, x1, [sp], 16

	eret
