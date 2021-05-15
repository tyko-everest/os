.global _start
.global _delay

.section .init
_start:
	// disable the hypervisor, execution should never get above EL1 now
	// unless the kernel issues a HVC instruction, but it won't
	// bit 31 set means use aarch64 for lower ELs
	mov x0, 1 << 31
	msr HCR_EL2, x0

	// don't trap any floating point instructions
	mov x0, 0b11 << 20
	msr CPACR_EL1, x0

	// setup up vector table base address for EL1
	adr x0, _vector
	msr VBAR_EL1, x0

	// setup SCTLR_EL1
	msr SCTLR_EL1, xzr

	// setup saved mode as EL1, using SP_EL1
	mov x0, #0b0101
	msr SPSR_EL2, x0

	// set "return point" to just below after eret
	adr x0, _el1_entry
	msr ELR_EL2, x0
	// move into EL1
	eret
_el1_entry:
	// stack must be 16-bytes aligned
	ldr x0, =0x20000000
	mov sp, x0

	// zero bss
    ldr x1, =__bss_start
    ldr x2, =__bss_end
_bss_loop:
    str xzr, [x1], #8
    cmp x1, x2
    blt _bss_loop

	bl main
_infinite_loop:
	wfe
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


_enter_el0:
	msr SPSR_EL1, xzr
	// set execution to just after eret
	adr x0, _in_el0
	msr ELR_EL1, x0
	mrs x0, CurrentEL
	// move into EL0
	eret
_in_el0:
	svc #1
	ret
	