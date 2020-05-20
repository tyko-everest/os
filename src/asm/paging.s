section .text

; invalidate_tlb - invalidate page in tlb
; stack: [esp + 4] the page that handles this virtual address will be cleared
;        [esp    ] the return address
global invalidate_tlb
invalidate_tlb:
    invlpg[esp + 4]
    ret