#include "stdint.h"
#include "print.h"
#include "gdt.h"
#include "interrupts.h"
#include "kheap.h"
#include "ata.h"

#include "multiboot.h"

void delay() {
    for (int i = 0; i < 1000000; i++);
}

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


    static uint32_t buffer[128];
    for (int i = 0; i < 128; i++) {
        buffer[i] = 0;
    }
    ata_pio_read(ATA0, ATA_MASTER, 0, 1, buffer);

    print_uint(*(uint32_t*)buffer);

    while (1);
}