#include "stdint.h"
#include "print.h"
#include "gdt.h"
#include "interrupts.h"
#include "memory.h"

#include "multiboot.h"

void delay() {
    for (int i = 0; i < 1000000; i++);
}

void kmain(multiboot_info_t* mbt, unsigned int magic) {
    // setup flat memory model for kernel
    setup_flat_memory();
    // necessary to use print to serial or fb
    print_init();
    // necessary to setup and enable interrupts
    interrupts_init();

    kheap_init();

    uint32_t *test1 = kmalloc(4 * sizeof(uint32_t));
    uint32_t *test2 = kmalloc(sizeof(uint32_t));
    uint8_t *test3 = kmalloc(sizeof(uint8_t));

    kfree(test2);
    print_kmem_blocks();
    print_nl();
    kfree(test1);
    print_kmem_blocks();
    kfree(test3);
    print_kmem_blocks();
    print_nl();

    // //find what physical memory is free
    // init_free_memory(mbt);    

    while (1);
}