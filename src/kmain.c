#include "stdint.h"
#include "string.h"
#include "print.h"
#include "gdt.h"
#include "interrupts.h"
#include "kheap.h"
#include "ata.h"
#include "page_frame.h"
#include "file_system.h"
#include "shell.h"

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
    // setup file system
    fs_init();
    shell_init();

    fs_inode_t root_inode = fs_get_inode(UXT_ROOT_INO);
    fs_ls(&root_inode);

    static uint8_t buf[2048];
    // fs_mkfile("TEST", UXT_ROOT_INO, &root_inode, UXT_FREG, 0, 0);
    // for (uint32_t i = 0; i < 2048; i++) {
    //     buf[i] = i % 256;
    // }
    // fs_writefile("TEST", UXT_ROOT_INO, &root_inode, 0, 2, buf);
    // memset(buf, 0, 2048);
    fs_readfile("TEST", UXT_ROOT_INO, &root_inode, 0, 2, buf);

    // copy program code and data to virtual address 0
    allocate_page(0, PRESENT | READ_WRITE | USER_ACCESS);
    uint8_t *ptr = (uint8_t *) 0;
    memcpy(ptr, user_prog, USER_PROG_LEN);
    // allocate stack for program at end of 3 GiB lower half
    allocate_page(0xC0000000 - 0x1000, PRESENT | READ_WRITE | USER_ACCESS);
    // enter user mode
    //syscall_test();
    enter_user_mode();



   



    while (1);
}