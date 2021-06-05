.global _start
.section .init

.macro clear_regs
	mov x\@, xzr
.endm

_start:
	// note: SCTLR_EL2 is already setup by armstub8.s

	// disable the hypervisor, execution should never get above EL1 now
	// unless the kernel issues a HVC instruction, but it won't
	// bit 31 set means use aarch64 for lower ELs
	mov x0, 1 << 31
	msr HCR_EL2, x0

	// zero bss
	ldr x4, =__virtual_start
    ldr x1, =__bss_start
	sub x1, x1, x4
    ldr x2, =__bss_end
	sub x2, x2, x4
_bss_loop:
    str xzr, [x1], #8
    cmp x1, x2
    blt _bss_loop

	// don't trap any floating point instructions
	mov x0, 0b11 << 20
	msr CPACR_EL1, x0

	// setup up vector table base address for EL1
	ldr x0, =_vector
	msr VBAR_EL1, x0

	// setup saved mode as EL1, using SP_EL1
	ldr x0, =0x000003c5
	msr SPSR_EL2, x0
	// set "return point" to just below after eret
	ldr x0, =_el1_entry
	sub x0, x0, x4
	msr ELR_EL2, x0
	// move into EL1
	eret
_el1_entry:

	// setup the mmu

	// setup 4 KB granules in TTBR1 and TTBR0, and 25 MSB not checked for both initially
	// i.e. 512 MB VM space
	ldr x0, =(0b10 << 30) | (0b11 << 28) | (0b0101 << 24) | (25 << 16) | (0b11 << 12) | (0b0101 << 8) | 25
	msr TCR_EL1, x0

	// setup memory attribute 0 as normal memory, Inner and Outer Write-Back Non-transient
	// setup memory attribute 1 as device memory, nGnRE 
	mov x0, 0x04FF
	msr MAIR_EL1, x0

	/*
	TODO

	Want to not use the entire virtual memory space for the kernel,
	the kernel will never be swapped out so no point in it being bigger than physical memory
	will likely drop down to only 2nd and 3rd level translation, this just maps 1 GB

	Note! The higher virtual addresses always end at FFFF FFFF FFFF FFFF
	This means by decreases the space, need to link above FFFF 0000 0000 0000
	i.e. if space were 1GB, higher VM starts at FFFF FFFF C000 0000
	*/

	// flat map the entire 1 GB of physical memory at 0xFFFF000000000000
	// the first entry of this stage 1 table controls this memory
	ldr x0, =_tt_lv1
	sub x0, x0, x4
	msr TTBR0_EL1, x0
	msr TTBR1_EL1, x0
	mov x1, 0 // physical memory starts at 0
	mov x2, 0x701 // should be good config for RW in EL1
	orr x1, x1, x2
	str x1, [x0]
	// map the next 1GB as the device memory (even though its only 16 MB)
	// mistake! if output area ia 1GB aligned, input phys mem needs to be 1GB aligned as well!
	// this is only 16 MB aligned, here is the issue (at least I really hope so)
	mov x1, 0
	mov x2, 0x605
	orr x1, x1, x2
	str x1, [x0, 8]
	dsb sy
	isb

	// enable the mmu, no caches for now
	mrs x0, SCTLR_EL1
	orr x0, x0, 1
	msr SCTLR_EL1, x0
	dsb sy
	isb

	ldr x0, =_virt_mapping
	br x0
_virt_mapping:
	// operating in the high VM range now
	// stack must be 16-bytes aligned
	// will start it below the text; it can overwrite the earlier bootcode
	ldr x0, =__text_start
	mov sp, x0

	bl main
_infinite_loop:
	wfe
	b _infinite_loop

.section .bss
.global _tt_lv1
.balign 4096
.lcomm _tt_lv1, 4096
