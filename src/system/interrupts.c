#include "interrupts.h"


static idt_entry_t idt[NUM_INTERRUPTS];
static idt_header_t idt_info;

void register_idt_entry(unsigned int interrupt, unsigned int isr_addr, unsigned int ring) {
    if (interrupt >= NUM_INTERRUPTS) {
        return;
    }
    if (ring > 3) {
        return;
    }
    idt[interrupt].offset_high = (unsigned short) ((isr_addr >> 16) & 0xFFFF);
    idt[interrupt].offset_low = (unsigned short ) (isr_addr & 0xFFFF);
    // offset of the cs segment from the gdt
    idt[interrupt].segment = 0x08;
    // run at PL0
    idt[interrupt].flags = 0x8E | (ring << 5);
    idt[interrupt].zero = 0;
}

void interrupts_init() {
    idt_info.size = sizeof(idt_entry_t) * NUM_INTERRUPTS - 1;
    idt_info.address = idt;
    
    // remap the PICs to not conflict with other interrupts
    pic_remap();

    // register all interrupts in the idt
    register_idt_entry(0, (unsigned int) interrupt_handler_0, 0);
    register_idt_entry(1, (unsigned int) interrupt_handler_1, 0);
    register_idt_entry(2, (unsigned int) interrupt_handler_2, 0);
    register_idt_entry(3, (unsigned int) interrupt_handler_3, 0);
    register_idt_entry(4, (unsigned int) interrupt_handler_4, 0);
    register_idt_entry(5, (unsigned int) interrupt_handler_5, 0);
    register_idt_entry(6, (unsigned int) interrupt_handler_6, 0);
    register_idt_entry(7, (unsigned int) interrupt_handler_7, 0);
    register_idt_entry(8, (unsigned int) interrupt_handler_8, 0);
    register_idt_entry(9, (unsigned int) interrupt_handler_9, 0);
    register_idt_entry(10, (unsigned int) interrupt_handler_10, 0);
    register_idt_entry(11, (unsigned int) interrupt_handler_11, 0);
    register_idt_entry(12, (unsigned int) interrupt_handler_12, 0);
    register_idt_entry(13, (unsigned int) interrupt_handler_13, 0);
    register_idt_entry(14, (unsigned int) interrupt_handler_14, 0);
    register_idt_entry(15, (unsigned int) interrupt_handler_15, 0);
    register_idt_entry(16, (unsigned int) interrupt_handler_16, 0);
    register_idt_entry(17, (unsigned int) interrupt_handler_17, 0);
    register_idt_entry(18, (unsigned int) interrupt_handler_18, 0);
    register_idt_entry(19, (unsigned int) interrupt_handler_19, 0);
    register_idt_entry(20, (unsigned int) interrupt_handler_20, 0);
    register_idt_entry(21, (unsigned int) interrupt_handler_21, 0);
    register_idt_entry(22, (unsigned int) interrupt_handler_22, 0);
    register_idt_entry(23, (unsigned int) interrupt_handler_23, 0);
    register_idt_entry(24, (unsigned int) interrupt_handler_24, 0);
    register_idt_entry(25, (unsigned int) interrupt_handler_25, 0);
    register_idt_entry(26, (unsigned int) interrupt_handler_26, 0);
    register_idt_entry(27, (unsigned int) interrupt_handler_27, 0);
    register_idt_entry(28, (unsigned int) interrupt_handler_28, 0);
    register_idt_entry(29, (unsigned int) interrupt_handler_29, 0);
    register_idt_entry(30, (unsigned int) interrupt_handler_30, 0);
    register_idt_entry(31, (unsigned int) interrupt_handler_31, 0);
    // PIC1
    register_idt_entry(INT_PIC1_TIMER,
        (unsigned int) interrupt_handler_32, 0);
    register_idt_entry(INT_PIC1_KEYBOARD,
        (unsigned int) interrupt_handler_33, 0);
    // syscall
    register_idt_entry(128, (unsigned int) interrupt_handler_128, 3);

    // load the idt into the cpu
    load_idt(idt_info);

    // enable the interrupts we want in the pic
    // right now this is just the keyboard
    pic_set_mask(1, ~(1 << 1));
    
    enable_interrupts();

}

extern int cmd_entered;

void interrupt_handler(bool inter_privilege, stack_state_t stack, 
        unsigned int interrupt, cpu_state_t cpu) {
    
    char val_str[2];
    val_str[0] = 0;
    val_str[1] = 0;

    switch (interrupt) {
        case INT_PIC1_KEYBOARD:
            // let the keyboard driver know it should get the next scan code
            keyboard_int();
            pic_acknowledge(interrupt);
            break;

        case INT_SYSCALL:
            print("syscall\n", IO_OUTPUT_FB);
            syscall_handler(stack.eax, stack.ebx, stack.ecx, stack.edx);
            break;

        default:
            print("unhandled interrupt\n", IO_OUTPUT_FB);
            while(1);
            break;
    }
}

void pic_set_mask(unsigned int pic, unsigned char mask) {
    // writing to data port sets the interrupt mask
    switch (pic) {
    case 1:
        outb(PIC1_DATA_PORT, mask);
        break;
    
    case 2:
        outb(PIC2_DATA_PORT, mask);
        break;

    default:
        break;
    }
}

unsigned char pic_read_mask(unsigned int pic) {
    // reading from the PIC data port gets the interrupt mask
    switch (pic) {
    case 1:
        return inb(PIC1_DATA_PORT);
        break;
    
    case 2:
        return inb(PIC2_DATA_PORT);
        break;

    default:
        return 0;
        break;
    }
}

void pic_acknowledge(unsigned int interrupt) {
    if (interrupt < PIC1_START_INTERRUPT || interrupt > PIC2_END_INTERRUPT) {
        return;
    }
    if (interrupt < PIC2_START_INTERRUPT) {
        outb(PIC1_COMMAND_PORT, PIC_ACK);
    } else {
        // might 
        outb(PIC2_COMMAND_PORT, PIC_ACK);
    }
}

void pic_remap(void) {
    // restart PIC1 and PIC2
    outb(PIC1_COMMAND_PORT, 0x11);
    outb(PIC2_COMMAND_PORT, 0x11);
    
    // PIC1 now starts at 32
    outb(PIC1_DATA_PORT, 0x20);
    // PIC2 now starts at 40
    outb(PIC2_DATA_PORT, 0x28);

    // setup cascading
    outb(PIC1_DATA_PORT, 0x04);
    outb(PIC2_DATA_PORT, 0x02);

    // done
    outb(PIC1_DATA_PORT, 0x01);
    outb(PIC2_DATA_PORT, 0x01);
}