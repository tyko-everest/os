section .text

; outb - send a byte to an I/O port
; stack: [esp + 8] the data byte
;        [esp + 4] the I/O port
;        [esp    ] return address
global outb
outb:
    mov al, [esp + 8]   ; move the data to be sent into the al register
    mov dx, [esp + 4]   ; move the address of the I/O port into the dx register
    out dx, al          ; send the data to the I/O port
    ret

; inb - receive a byte on an I/O port
; stack: [esp + 4] the I/O port
;        [esp    ] return address
; return: eax      the received byte
global inb
inb:
    mov dx, [esp + 4]   ; move the address of the I/O port into the dx register
    in al, dx           ; receive the data from the I/O port
    ret

; outw - send a word (2 bytes) to an I/O port
; stack: [esp + 8] the data word
;        [esp + 4] the I/O port
;        [esp    ] return address
global outw
outw:
    mov ax, [esp + 8]
    mov dx, [esp + 4]
    out dx, ax
    ret

; inw - receive a word (2 bytes) on an I/O port
; stack: [esp + 4] the I/O port
;        [esp    ] return address
; return: eax      the received byte
global inw
inw:
    mov dx, [esp + 4]
    in ax, dx
    ret