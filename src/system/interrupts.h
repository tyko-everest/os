#ifndef INCLUDE_IDT_H
#define INCLUDE_IDT_H

#include "port.h"
#include "keyboard.h"
#include "print.h"

#define NUM_INTERRUPTS 256

#define PIC1_COMMAND_PORT 0x20
#define PIC1_DATA_PORT    0x21
#define PIC2_COMMAND_PORT 0xA0
#define PIC2_DATA_PORT    0xA1

/* The PIC interrupts have been remapped */
#define PIC1_START_INTERRUPT 0x20
#define PIC1_END_INTERRUPT   (PIC1_START_INTERRUPT + 7)
#define PIC2_START_INTERRUPT 0x28
#define PIC2_END_INTERRUPT   (PIC2_START_INTERRUPT + 7)

// the various PIC interrupts we use */
#define INT_PIC1_TIMER (PIC1_START_INTERRUPT + 0x0)
#define INT_PIC1_KEYBOARD (PIC1_START_INTERRUPT + 0x1)

// number to send to PIC command port that issued interrupt
#define PIC_ACK 0x20

typedef struct __attribute__((packed)) {
    unsigned short size;
    unsigned int address;
} idt_header_t;

typedef struct __attribute__((packed)) {
    unsigned short offset_low;
    unsigned short segment;
    unsigned char zero;
    unsigned char flags;
    unsigned short offset_high;
} idt_entry_t;

typedef struct __attribute__((packed)) {
    // order is important, pusha pushes them in opposite of this order
    // so it needs to be reversed
    unsigned int edi;
    unsigned int esi;
    unsigned int ebp;
    unsigned int esp;
    unsigned int ebx;
    unsigned int edx;
    unsigned int ecx;
    unsigned int eax;
} stack_state_t;

typedef struct __attribute__((packed)) {
    unsigned int error_code;
    unsigned int eip;
    unsigned int cs;
    unsigned int eflags;
} cpu_state_t;

void enable_interrupts(void);
void disable_interrupts(void);

// system interrupts
void interrupt_handler_0(void);
void interrupt_handler_1(void);
void interrupt_handler_2(void);
void interrupt_handler_3(void);
void interrupt_handler_4(void);
void interrupt_handler_5(void);
void interrupt_handler_6(void);
void interrupt_handler_7(void);
void interrupt_handler_8(void);
void interrupt_handler_9(void);
void interrupt_handler_10(void);
void interrupt_handler_11(void);
void interrupt_handler_12(void);
void interrupt_handler_13(void);
void interrupt_handler_14(void);
void interrupt_handler_15(void);
void interrupt_handler_16(void);
void interrupt_handler_17(void);
void interrupt_handler_18(void);
void interrupt_handler_19(void);
void interrupt_handler_20(void);
void interrupt_handler_21(void);
void interrupt_handler_22(void);
void interrupt_handler_23(void);
void interrupt_handler_24(void);
void interrupt_handler_25(void);
void interrupt_handler_26(void);
void interrupt_handler_27(void);
void interrupt_handler_28(void);
void interrupt_handler_29(void);
void interrupt_handler_30(void);
void interrupt_handler_31(void);

// hardware interrupts
void interrupt_handler_32(void);
void interrupt_handler_33(void);

// used to reference the assembly wrapper for the c handler
void pic1_keyboard_handler_entry(void);

void register_idt_entry(unsigned int interrupt, unsigned int isr_addr);

void load_idt(idt_header_t idt_info);

void interrupts_init(void);

void interrupt_handler(stack_state_t stack, unsigned int interrupt, cpu_state_t cpu);

/** pic_acknowledge:
 *  Acknowledges an interrupt from either PIC 1 or PIC 2.
 *
 *  @param num The number of the interrupt
 */
void pic_acknowledge(unsigned int interrupt);

void pic_remap(void);

void pic_set_mask(unsigned int pic, unsigned char mask);

unsigned char pic_read_mask(unsigned int pic);

#endif /* INCLUDE_IDT_H */