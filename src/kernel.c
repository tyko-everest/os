#include "arch/registers.h"
#include "clib/stdint.h"
#include "drivers/mmio.h"
#include "drivers/serial.h"
#include "utils/printf.h"
#include "system/kheap.h"
#include "system/fat32.h"
#include "system/pm.h"
#include "system/mmu.h"
#include "system/syscall.h"
#include "utils/elf.h"

extern void * __files_start;

uint32_t _get_sctlr();

ssize_t read(const char *path, void *buf, size_t count, size_t from) {
    asm("svc 0");
}

int main() {

    serial_init();
    init_printf(NULL, serial_putc);
    printf("\nprintf initialized\n");
    kheap_init();
    printf("kernel heap initialized\n");
    // fat32_init();
    // printf("fat32 initialized\n");
    pm_init();
    printf("phys memory initialized\n");
    vm_init();
    printf("virt memory initialized\n");

    uintptr_t pm = pm_get_page();
    volatile int res1 = vm_allocate_page(0, pm, VM_ACCESS_FLAG | VM_SHARE_INNER | VM_KERNEL_RW);
    
    

    // const char path[] = "/PRG";
    // elf64_header_t header;
    // elf64_prog_header_t prog_header;
    // parse_elf(path, &header, &prog_header);

    // char buf[100];

    // printf("making read\n");
    // volatile int res = read(path, buf, 100, 0);
    // printf("read from file: %s\n", buf);

    printf("looping forever...\n");
    for(;;);
    return 0;
}