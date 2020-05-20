global load_gdt
global load_tss
global load_segment_registers

section .text

; load_gdt - load a gdt struct into the cpu
; stack: [esp + 4] the physical address of the gdt info struct
;        [esp    ] the return address
load_gdt:
    lgdt [esp + 4]  ; load the gdt at the given address
    ret

; load_tss - load a tss struct into cpu
; stack: [esp + 4] the segment selector
;        [esp] the return address
load_tss:
    ltr [esp + 4]
    ret

; load_segment_registers - load the segment registers with the correct
; values from the gdt
; stack: [esp] the return address 
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
