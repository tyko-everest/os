%macro no_error_code_interrupt_handler 1
global interrupt_handler_%1
interrupt_handler_%1:
    push dword 0                    ; push 0 as error code
    push dword %1                   ; push interrupt number
    jmp common_interrupt_handler    ; jump to common handler
%endmacro

%macro error_code_interrupt_handler 1
global interrupt_handler_%1
interrupt_handler_%1:
    push dword %1                   ; push interrupt number
    jmp common_interrupt_handler    ; jump to common handler
%endmacro

extern interrupt_handler

section .text

common_interrupt_handler:

    ; save the registers
    push ds
    pushad

    ; we need to determine if this has been an interprivilege interrupt
    ; get old cs from stack
    mov eax, [esp + 12*4] ; skip 9 registers + err code + isr# + eip
    ; get new cs
    mov ebx, cs
    ; compare the two
    cmp eax, ebx
    jne inter_privil
    push 0
    jmp done_privil_cmp
inter_privil:
    push 1
done_privil_cmp:

    ; call c function to handle the rest
    call interrupt_handler
    ; remove the the inter_privilege bool
    add esp, 4
    ; restore the registers
    popad
    pop ds
    
    ; add 8 due to error code and interrupt num pushed earlier
    ; stack must be exactly how it was when it entered the ISR
    ; error code must be off the stack
    add esp, 8

    ; re-enable interrupts
    sti
    iret

; define all the used interrupt handlers here
; cpu interrupts
no_error_code_interrupt_handler 0
no_error_code_interrupt_handler 1
no_error_code_interrupt_handler 2
no_error_code_interrupt_handler 3
no_error_code_interrupt_handler 4
no_error_code_interrupt_handler 5
no_error_code_interrupt_handler 6
no_error_code_interrupt_handler 7
error_code_interrupt_handler    8
no_error_code_interrupt_handler 9
error_code_interrupt_handler    10
error_code_interrupt_handler    11
error_code_interrupt_handler    12
error_code_interrupt_handler    13
error_code_interrupt_handler    14
no_error_code_interrupt_handler 15
no_error_code_interrupt_handler 16
error_code_interrupt_handler    17
no_error_code_interrupt_handler 18
no_error_code_interrupt_handler 19
no_error_code_interrupt_handler 20
no_error_code_interrupt_handler 21
no_error_code_interrupt_handler 22
no_error_code_interrupt_handler 23
no_error_code_interrupt_handler 24
no_error_code_interrupt_handler 25
no_error_code_interrupt_handler 26
no_error_code_interrupt_handler 27
no_error_code_interrupt_handler 28
no_error_code_interrupt_handler 29
no_error_code_interrupt_handler 30
no_error_code_interrupt_handler 31

; pic interrupts
; timer
no_error_code_interrupt_handler 32
; keyboard
no_error_code_interrupt_handler 33

; syscall
no_error_code_interrupt_handler 128

global load_idt
; load_idt - Loads the interrupt descriptor table (IDT).
; stack: [esp + 4] the address of the IDT descriptor
;        [esp    ] the return address
load_idt:
    lidt [esp + 4]          ; load the IDT
    ret                     ; return to the calling function

global disable_interrupts
disable_interrupts:
    cli
    ret

global enable_interrupts
enable_interrupts:
    sti
    ret

global enter_user_mode
enter_user_mode:
    mov ax, 0x20 | 0x03
    mov ds, ax
    ; mov es, ax
    ; mov fs, ax
    ; mov gs, ax
    push 0x20 | 0x3         ; stack segment for user data, and ring 3
    push 0xC0000000 - 4     ; base stack address
    push 0                  ; eflags register
    push 0x18 | 0x3         ; code segment for user, and ring 3
    push 0                  ; eip for user program
    iret

global call_iret
call_iret:
    ; setup ds, hardcoded for user mode right now
    mov ax, 0x20 | 0x03
    mov ds, ax

    ; cpu_state_t struct has error code on it, can't stay on stack
    ; TODO find out why had to back up two values on the stack
    add esp, 8 
    iret

global syscall_test
syscall_test:
    int 0x80
    ret