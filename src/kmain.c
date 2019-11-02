#include "stdint.h"
#include "print.h"
#include "gdt.h"
#include "interrupts.h"
#include "kheap.h"

#include "multiboot.h"

void delay() {
    for (int i = 0; i < 1000000; i++);
}

void kmain(multiboot_info_t* mbt, unsigned int magic) {
    // setup flat memory model for kernel and user code
    setup_flat_memory();
    // necessary to use print to serial or fb
    print_init();
    // necessary to setup and enable interrupts
    interrupts_init();
    // kernel heap needs to be setup before using kmalloc
    kheap_init();
    //find what physical memory is free
    init_free_memory(mbt);

    

    print_free_memory();

    while (1);
}