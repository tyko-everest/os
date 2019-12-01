global load_gdt
global load_tss
global load_segment_registers

extern gdt

section .text

; load_gdt - load a gdt struct into the cpu
; stack: [esp + 4] the gdt info struct
;        [esp    ] the return address
load_gdt:
    lgdt [esp + 4]  ; load the gdt at the given address
    ret

; load_tss - load a tss struct into cpu
; stack: [esp] + 4] the tss struct
load_tss:
    mov ax, 0x28
    ltr ax

; load_segment_registers - load the segment registers with the correct
; values from the gdt
; stack :   [esp + 4] the address of the null gdt_entry
;           [esp] the return address 
load_segment_registers:
    mov ax, 0x10    ; get the correct offset for the data segment
    mov ds, ax
    mov ss, ax
    mov es, ax 
    mov gs, ax
    mov fs, ax
    jmp 0x08:flush_cs
flush_cs:
    ret
