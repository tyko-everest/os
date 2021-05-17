.global _start
.section .init
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

	// setup the mmu

	// setup 4 KB granules in TTBR1 and TTBR0, and 16 MSB not checked
	ldr x0, =(0b10 << 30) | (0b11 << 28) | (0b0101 << 24) | (16 << 16) | (0b11 << 12) | (0b0101 << 8) | 16
	msr TCR_EL1, x0

	// setup memory attribute 0 as normal memory, Inner and Outer Write-Back Non-transient
	// setup memory attribute 1 as device memory, nGnRnE 
	mov x0, 0x00FF
	msr MAIR_EL1, x0

	// setting up translation table TTBR1_EL1
	// setup the first entry of the level 0 translation table
	// map it into the first level 1 translation table
	// this maps the first 512 GB of VM
	ldr x0, =_tt_lv0
	sub x0, x0, x4
	// table entry, not block entry
	mov x1, 0b11
	// the address of the table
	ldr x2, =_tt_lv1
	sub x2, x2, x4
	orr x1, x1, x2
	// save this configuration in the table
	str x1, [x0]
	// finally save this setup in TTBR1
	msr TTBR1_EL1, x0
	
	// flat map the entire 1 GB of physical memory at 0xFFFF000000000000
	// the first entry of this stage 1 table controls this memory
	ldr x0, =_tt_lv1
	sub x0, x0, x4
	ldr x1, =0x0 // physical memory starts at 0
	mov x2, 0x701 // should be good config for RW in EL1
	orr x1, x1, x2
	str x1, [x0], #8
	// map the next 1GB as the device memory (even though its only 16 MB)
	ldr x1, =0x3F000000
	mov x2, 0x405
	orr x1, x1, x2
	str x1, [x0]
	dsb sy
	isb

	// enable the mmu
	tlbi ALLE1
	mrs x0, SCTLR_EL1
	//mov x1, ((1 << 12) | (1 << 2) | 1)
	mov x1, 1
	orr x0, x0, x1
	msr SCTLR_EL1, x0
	dsb sy
	isb

	// setup saved mode as EL1, using SP_EL1
	mov x0, #0b0101
	msr SPSR_EL2, x0

	// set "return point" to just below after eret
	// this also transfers control to using virtual addresses
	ldr x0, =_el1_entry
	msr ELR_EL2, x0
	// move into EL1
	eret
_el1_entry:
	// also operating in the high 48 bit VM range now, as MMU is enabled in EL1
	// stack must be 16-bytes aligned
	ldr x0, =0xFFFF000000080000
	mov sp, x0

	bl main
_infinite_loop:
	wfe
	b _infinite_loop

.section .text

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

.section .bss
.global _tt_lv0
.global _tt_lv1
.balign 4096
.lcomm _tt_lv0, 4096
.balign 4096
.lcomm _tt_lv1, 4096
