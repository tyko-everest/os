.global _vector

.equ reg_stack_frame, 32*8 + 32*16

.section .vector
.balign 2048
_vector:

	// current EL with SP0
.balign 128
	// synch
	sub sp, sp, reg_stack_frame
	stp x0, x1, [sp]
	mov x0, 0
	b _vector_common
.balign 128
	// IRQ
	sub sp, sp, reg_stack_frame
	stp x0, x1, [sp]
	mov x0, 1
	b _vector_common
.balign 128
	// FIQ
	sub sp, sp, reg_stack_frame
	stp x0, x1, [sp]
	mov x0, 2
	b _vector_common
.balign 128
	// SError
	sub sp, sp, reg_stack_frame
	stp x0, x1, [sp]
	mov x0, 3
	b _vector_common

	// current EL with SPx
.balign 128
	// synch
	sub sp, sp, reg_stack_frame
	stp x0, x1, [sp]
	mov x0, 4
	b _vector_common
.balign 128
	// IRQ
	sub sp, sp, reg_stack_frame
	stp x0, x1, [sp]
	mov x0, 5
	b _vector_common
.balign 128
	// FIQ
	sub sp, sp, reg_stack_frame
	stp x0, x1, [sp]
	mov x0, 6
	b _vector_common
.balign 128
	// SError
	sub sp, sp, reg_stack_frame
	stp x0, x1, [sp]
	mov x0, 7
	b _vector_common

	// lower EL with AArch64
.balign 128
	// synch
	sub sp, sp, reg_stack_frame
	stp x0, x1, [sp]
	mov x0, 8
	b _vector_common
.balign 128
	// IRQ
	sub sp, sp, reg_stack_frame
	stp x0, x1, [sp]
	mov x0, 9
	b _vector_common
.balign 128
	// FIQ
	sub sp, sp, reg_stack_frame
	stp x0, x1, [sp]
	mov x0, 10
	b _vector_common
.balign 128
	// SError
	sub sp, sp, reg_stack_frame
	stp x0, x1, [sp]
	mov x0, 11
	b _vector_common

	// lower EL with AArch32
.balign 128
	// synch
	sub sp, sp, reg_stack_frame
	stp x0, x1, [sp]
	mov x0, 12
	b _vector_common
.balign 128
	// IRQ
	sub sp, sp, reg_stack_frame
	stp x0, x1, [sp]
	mov x0, 13
	b _vector_common
.balign 128
	// FIQ
	sub sp, sp, reg_stack_frame
	stp x0, x1, [sp]
	mov x0, 14
	b _vector_common
.balign 128
	// SError
	sub sp, sp, reg_stack_frame
	stp x0, x1, [sp]
	mov x0, 15
	b _vector_common

_vector_common:
	stp x2, x3, [sp, 16*1]
	stp x4, x5, [sp, 16*2]
	stp x6, x7, [sp, 16*3]
	stp x8, x9, [sp, 16*4]
	stp x10, x11, [sp, 16*5]
	stp x12, x13, [sp, 16*6]
	stp x14, x15, [sp, 16*7]
	stp x16, x17, [sp, 16*8]
	stp x18, x19, [sp, 16*9]
	stp x20, x21, [sp, 16*10]
	stp x22, x23, [sp, 16*11]
	stp x24, x25, [sp, 16*12]
	stp x26, x27, [sp, 16*13]
	stp x28, x29, [sp, 16*14]
	stp x30, xzr, [sp, 16*15]

	stp q0, q1, [sp, 16*16 + 32*0]
	stp q2, q3, [sp, 16*16 + 32*1]
	stp q4, q5, [sp, 16*16 + 32*2]
	stp q6, q7, [sp, 16*16 + 32*3]
	stp q8, q9, [sp, 16*16 + 32*4]
	stp q10, q11, [sp, 16*16 + 32*5]
	stp q12, q13, [sp, 16*16 + 32*6]
	stp q14, q15, [sp, 16*16 + 32*7]
	stp q16, q17, [sp, 16*16 + 32*8]
	stp q18, q19, [sp, 16*16 + 32*9]
	stp q20, q21, [sp, 16*16 + 32*10]
	stp q22, q23, [sp, 16*16 + 32*11]
	stp q24, q25, [sp, 16*16 + 32*12]
	stp q26, q27, [sp, 16*16 + 32*13]
	stp q28, q29, [sp, 16*16 + 32*14]
	stp q30, q31, [sp, 16*16 + 32*15]

	mrs x1, ESR_EL1
	mov x2, sp
	
	bl interrupt_handler
	
	ldp x0, x1, [sp]
	ldp x2, x3, [sp, 16*1]
	ldp x4, x5, [sp, 16*2]
	ldp x6, x7, [sp, 16*3]
	ldp x8, x9, [sp, 16*4]
	ldp x10, x11, [sp, 16*5]
	ldp x12, x13, [sp, 16*6]
	ldp x14, x15, [sp, 16*7]
	ldp x16, x17, [sp, 16*8]
	ldp x18, x19, [sp, 16*9]
	ldp x20, x21, [sp, 16*10]
	ldp x22, x23, [sp, 16*11]
	ldp x24, x25, [sp, 16*12]
	ldp x26, x27, [sp, 16*13]
	ldp x28, x29, [sp, 16*14]
	ldp x30, xzr, [sp, 16*15]

	ldp q0, q1, [sp, 16*16 + 32*0]
	ldp q2, q3, [sp, 16*16 + 32*1]
	ldp q4, q5, [sp, 16*16 + 32*2]
	ldp q6, q7, [sp, 16*16 + 32*3]
	ldp q8, q9, [sp, 16*16 + 32*4]
	ldp q10, q11, [sp, 16*16 + 32*5]
	ldp q12, q13, [sp, 16*16 + 32*6]
	ldp q14, q15, [sp, 16*16 + 32*7]
	ldp q16, q17, [sp, 16*16 + 32*8]
	ldp q18, q19, [sp, 16*16 + 32*9]
	ldp q20, q21, [sp, 16*16 + 32*10]
	ldp q22, q23, [sp, 16*16 + 32*11]
	ldp q24, q25, [sp, 16*16 + 32*12]
	ldp q26, q27, [sp, 16*16 + 32*13]
	ldp q28, q29, [sp, 16*16 + 32*14]
	ldp q30, q31, [sp, 16*16 + 32*15]	

	add sp, sp, reg_stack_frame
	eret
