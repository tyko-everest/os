#include "clib/stdint.h"
#include "clib/string.h"
#include "system/gdt.h"
#include "system/interrupts.h"
#include "system/kheap.h"
#include "system/page_frame.h"
#include "system/file_system.h"
#include "system/proc.h"
#include "system/multiboot.h"
#include "utils/print.h"
#include "utils/shell.h"

#include "system/syscall.h"


void delay() {
    for (int i = 0; i < 1000000; i++);
}

// used to load an elf file onto disk for testing
#define USER_PROG_LEN (1024*4) //4788
const uint8_t user_prog[USER_PROG_LEN] = {
0xb8, 0x01, 0x00, 0x00, 0x00, 0xbb, 0x18, 0x00, 0x00, 0x00,
0xb9, 0x20, 0x00, 0x00, 0x00, 0xba, 0x01, 0x00, 0x00, 0x00,
0xcd, 0x80, 0xeb, 0xfe, 0x2f, 0x54, 0x45, 0x53, 0x54, 0x00,
0x90, 0x90
};

#define TEST_LEN 1024
const uint8_t test_arr[TEST_LEN] = {
1, 2, 3, 4, 5, 6, 7, 8
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

    static uint8_t buf[1024 * 4];

    // fs_mkfile("ELF", UXT_ROOT_INO, &root_inode, UXT_FREG, 0, 0);
    fs_inode_t root_inode = fs_get_inode(1);
    fs_ls(&root_inode);
    //fs_writefile("/TEST", 0, 1, test_arr);
    //fs_readfile("/TEST", 0, 1, buf);

    process_t proc;
    proc_load("/ELF", &proc);
    // proc load isnt working, this is a temp replacement
    memcpy(0, user_prog, USER_PROG_LEN);

    // uint8_t *test = 32; 
    // sys_read(24, 32, 1);

    proc_start(&proc);
    

    int i = 'h';





    /*
    // copy program code and data to virtual address 0
    allocate_page(0, PRESENT | READ_WRITE | USER_ACCESS);
    uint8_t *ptr = (uint8_t *) 0;
    memcpy(ptr, user_prog, USER_PROG_LEN);
    // allocate stack for program at end of 3 GiB lower half
    allocate_page(0xC0000000 - 0x1000, PRESENT | READ_WRITE | USER_ACCESS);
    // enter user mode
    //syscall_test();
    enter_user_mode();
    */


   



    while (1);
}