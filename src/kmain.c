#include "stdint.h"
#include "print.h"
#include "gdt.h"
#include "interrupts.h"
#include "kheap.h"
#include "ata.h"
#include "page_frame.h"

#include "multiboot.h"

void delay() {
    for (int i = 0; i < 1000000; i++);
}

// used to test making a syscall and returning afterwards
#define USER_PROG_LEN 14
uint8_t user_prog[USER_PROG_LEN] = {
    0xb8, 0x69, 0x00, 0x00, 0x00, 0xcd, 0x80, 0xb8, 0x20, 0x04,
    0x00, 0x00, 0xeb, 0xfe
};

void kmain(multiboot_info_t* mbt, uint32_t magic, uint32_t kernel_stack_base) {
    // setup flat memory model for kernel and user code
    setup_flat_memory();
    // setup the tss, so inter priviledge interrupts will work
    setup_tss(kernel_stack_base);
    // necessary to use print to serial or fb
    print_init();
    // necessary to setup and enable interrupts
    interrupts_init();
    // kernel heap needs to be setup before using kmalloc
    kheap_init();
    // find what physical memory is free
    init_free_memory(mbt);

    // copy program code to virtual address 0
    allocate_page(0, PRESENT | READ_WRITE | USER_ACCESS);
    uint8_t *ptr = (uint8_t *) 0;
    for (int i = 0; i < USER_PROG_LEN; i++) {
        ptr[i] = user_prog[i];
    }
    // allocate stack for program at end of 3 GiB lower half
    allocate_page(0xC0000000 - 0x1000, PRESENT | READ_WRITE | USER_ACCESS);
    // enter user mode
    //syscall_test();
    enter_user_mode();

   

    static uint32_t buffer[128];
    for (int i = 0; i < 128; i++) {
        buffer[i] = 0;
    }
    //ata_pio_read(ATA0, ATA_MASTER, 0, 1, buffer);

    while (1);
}