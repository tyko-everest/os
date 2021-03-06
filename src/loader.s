global loader                       ; the entry symbol for ELF

MAGIC_NUMBER    equ 0x1BADB002      ; define the magic number constant
FLAGS           equ 0x0             ; multiboot flags
CHECKSUM        equ -MAGIC_NUMBER   ; calculate the checksum
                                    ; (magic num + checksum + flags = 0)

extern kmain

; this is necessary to ensure it is within the first 8 kiB to avoid error 13
; https://wiki.osdev.org/Grub_Error_13
section .__mb_header
align 4
    dd MAGIC_NUMBER ; write the machine number to the machine code
    dd FLAGS        ; the flags
    dd CHECKSUM     ; the checksum

section .text       ; start of code segment
align 4             ; code must be 4 byte aligned
loader:             ; loader script defined as entry point in linker script
    mov esp, kernel_stack + KERNEL_STACK_SIZE   ; point esp to start of stack
                                                ; (end of memory segment)
    
    ; temporarily store grub magic number in ecx
    mov ecx, eax
    ; must take into account that paging will be setup when it is accessed
    mov eax, ebx
    add eax, KERNEL_VIRTUAL_BASE
    mov ebx, eax

enable_paging:
    ; first enable 4 MiB pages
    mov eax, cr4
    or eax, 0x00000010
    mov cr4, eax
    ; load kernel page directory
    ; needs to be the physical address
    mov eax, (kernel_page_directory - KERNEL_VIRTUAL_BASE)
    mov cr3, eax
    ; enable paging and protection bit
    mov eax, cr0
    or eax, 0x80000001
    mov cr0, eax
    ; jump to label to set eip to correct virtual address
    lea ecx, [.higher_half]
    jmp ecx
.higher_half:
    ; zero out the old identity mapping, no longer needed
    mov dword [kernel_page_directory], 0
    ; tell CPU the first entry has changed
    invlpg [0]

    ; push the base address of the stack onto the stack
    ; needed to setup the tss for interrupts
    push esp
    ; push the multiboot structures onto the stack
    push ecx
    push ebx
    call kmain      ; a c function!
.loop:
    jmp .loop       ; loop forever

; used for debugging
global magic_bp
magic_bp:
    xchg bx, bx
    ret


; page directories are 1024 entries
PAGE_DIRECTORY_ENTRIES equ 1024
; page directories are 4 kiB
PAGE_DIRECTORY_SIZE equ 4096
; we are running a higher half kernel
KERNEL_VIRTUAL_BASE equ 0xC0000000
; the entry number is the 10 msbs of the address
KERNEL_VIRTUAL_PAGE_NUMBER equ (KERNEL_VIRTUAL_BASE >> 22)
; bits 31 - 12: physical address of page table (not the page itself)
; flags (only what we care about)
; bit 7: 1 for 4 MiB page, ie. no pt for further breakdown
; bit 5: 1 to disable caching
; bit 2: 0 for kernel access only
; bit 1: 1 for read/write
; bit 0: 1 for present
; kernel and grub start at 0
KERNEL_PDE_FLAGS equ    0x00000083
PD_PDE_FLAGS equ        0x00000003

section .data
align 4096  ; page directories must be 4 KiB aligned
global kernel_page_directory
kernel_page_directory:
    ; identity map the kernel, grub, bios, etc.
    ; needed for setting up paging and zeroed out after
    ; they are at 0, so no need to combine addr with flags
    dd KERNEL_PDE_FLAGS
    ; fill in the entries between with 0
    times (KERNEL_VIRTUAL_PAGE_NUMBER - 1) dd 0;
    ; map as well at 3 GiB
    dd KERNEL_PDE_FLAGS;
    ; fill in the entries between with 0
    times (1024 - KERNEL_VIRTUAL_PAGE_NUMBER - 2) dd 0;
    ; map the last pde to the pd itself, so we can convert virt to phys addrs
    ; this is not stored at address 0 so need to add physical addr to flags
    dd (PD_PDE_FLAGS + kernel_page_directory - KERNEL_VIRTUAL_BASE)

align 4096
global temp_page_table_label
; used to temporarily store new page tables
temp_page_table_label:
    times 1024 dd 0

; 4 KiB kernel heap
; fixed size for now and pre-allocated
KERNEL_HEAP_SIZE equ 0x10000
KERNEL_STACK_SIZE equ 0x10000  ; in bytes

section .bss
align 4096
global kernel_heap
kernel_heap:
    resb KERNEL_HEAP_SIZE

align 4096
global kernel_stack
kernel_stack:
    resb KERNEL_STACK_SIZE  ; reserve stack for the kernel

align 
