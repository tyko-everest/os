.global _start
.section .init

.macro clear_regs
	mov x\@, xzr
.endm

_start:
	// note: SCTLR_EL2 is already setup by armstub8.s


	// temporarily setting up the serial as the first thing for debugging
	// setup gpio 14 and 15 as alt5
	movz x0, 0x3F20, lsl 16
	movk x0, 0x0004
	mov w1, 0b0010 << 12
	movk w1, 0b01, lsl 16
	str w1, [x0]
	// setup no pull up or down on these pins
	movk x0, 0x0094
	mov w1, 0
	str w1, [x0]
	.rept 150
	nop
	.endr
	movk x0, 0x0098
	mov w1, 0b11 << 14
	str w1, [x0]
	.rept 150
	nop
	.endr
	movk x0, 0x0094
	mov w1, 0
	str w1, [x0]
	movk x0, 0x0098
	mov w1, 0
	str w1, [x0]


	// setup uart
	mov x0, 0x5004
	movk x0, 0x3F21, lsl 16
	mov w1, 1
	str w1, [x0]
	// disable transmit and receiver
	movk x0, 0x5060
	mov w1, 0
	str w1, [x0]
	// enable 8 bit mode
	movk x0, 0x504C
	mov w1, 3
	str w1, [x0]
	// set baud rate of 115200
	movk x0, 0x5068
	mov w1, 270
	str w1, [x0]
	// enable transmit and receiver
	movk x0, 0x5060
	mov w1, 3
	str w1, [x0]
	// need delay before it becomes ready
	.rept 100
	nop
	.endr
	// setup x16 as IO_REG
	movk x0, 0x5040
	mov x16, x0

	mov w0, 'A'
	str w0, [x16]

	// disable the hypervisor, execution should never get above EL1 now
	// unless the kernel issues a HVC instruction, but it won't
	// bit 31 set means use aarch64 for lower ELs
	mov x0, 1 << 31
	msr HCR_EL2, x0

	mov w0, '1'
	str w0, [x16]

	// zero bss
	
	ldr x4, =__virtual_start
	mov x0, '2'
	str x0, [x16]
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
	// this also transfers control to using virtual addresses
	ldr x0, =_el1_entry
	sub x0, x0, x4
	msr ELR_EL2, x0
	// move into EL1
	eret
_el1_entry:

	mov x0, 'C'
	str x0, [x16]

	// setup the mmu

	// setup 4 KB granules in TTBR1 and TTBR0, and 16 MSB not checked
	ldr x0, =(0b10 << 30) | (0b11 << 28) | (0b0101 << 24) | (16 << 16) | (0b11 << 12) | (0b0101 << 8) | 16
	msr TCR_EL1, x0

	// setup memory attribute 0 as normal memory, Inner and Outer Write-Back Non-transient
	// setup memory attribute 1 as device memory, nGnRnE 
	mov x0, 0x04FF
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
	orr x0, x0, 1
	// finally save this setup in TTBR1
	msr TTBR1_EL1, x0
	msr TTBR0_EL1, x0
	
	// flat map the entire 1 GB of physical memory at 0xFFFF000000000000
	// the first entry of this stage 1 table controls this memory
	ldr x0, =_tt_lv1
	sub x0, x0, x4
	ldr x1, =0x0 // physical memory starts at 0
	mov x2, 0x701 // should be good config for RW in EL1
	orr x1, x1, x2
	str x1, [x0], #8
	// map the next 1GB as the device memory (even though its only 16 MB)
	// mistake! if output area ia 1GB aligned, input phys mem needs to be 1GB aligned as well!
	// this is only 16 MB aligned, here is the issue (at least I really hope so)
	ldr x1, =0 //=0x3F000000
	mov x2, 0x605
	orr x1, x1, x2
	str x1, [x0]
	dsb sy
	isb

	// enable the mmu, no caches for now
	dsb ish
	isb 
	mrs x0, SCTLR_EL1
	orr x0, x0, 1
	msr SCTLR_EL1, x0
	isb

	ldr x0, =_virt_mapping
	br x0
_virt_mapping:
	// now IO_REG will be at 0xFFFF000040215040
	ldr x16, =0xFFFF00007F215040
	mov x0, 'D'
	str x0, [x16]

/*
	// Disable L1 Caches
	MRS    X0, SCTLR_EL1 			// Read SCTLR_EL2. 
	BIC    X0,   X0, #(0x1 << 2)    // Disable D Cache.
	MSR    SCTLR_EL1, X0         	// Write SCTLR_EL3.
	// Invalidate Data cache to make the code general purpose.
	// Calculate the cache size first and loop through each set +
	//   way  . 
	MOV    X0, #0x0             //  X0 = Cache level  
	MSR    CSSELR_EL1, x0          // 0x0 for L1 Dcache  0x2    for L2 Dcache.
	MRS    X4, CCSIDR_EL1       // Read Cache Size ID.
	AND    X1, X4, #0x7
	ADD    X1, X1, #0x4         // X1 = Cache Line Size.
	LDR    X3, =0x7FFF  
	AND    X2, X3, X4, LSR   #13  // X2 = Cache Set Number – 1. 
	LDR    X3, =0x3FF 
	AND    X3, X3, X4, LSR   #3   //  X3 = Cache Associativity Number – 1. 
	CLZ    W4,   W3               // X4 = way position in the CISW  instruction.
	
	MOV    X5, #0               //  X5 = way counter way_loop.
way_loop:
	MOV    X6, #0               //  X6 = set counter set_loop.
set_loop:
	LSL    X7, X5,  X4 
	ORR    X7, X0, X7           // Set way. 
	LSL    X8, X6, X1 
	ORR    X7, X7,  X8           // Set set. 
	DC     cisw, X7             // Clean and Invalidate cache line.
	ADD    X6, X6, #1           // Increment set counter.
	CMP    X6, X2               // Last set reached yet?
	BLE    set_loop             // If not, iterate set_loop,
	ADD    X5, X5, #1           // else, next way.
	CMP    X5, X3               // Last way reached yet?
	BLE    way_loop             // I f not, iterate way_loop
	ic ialluis
*/

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
