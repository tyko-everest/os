#include "arch/registers.h"
#include "clib/stdint.h"
#include "drivers/mmio.h"
#include "drivers/serial.h"
#include "utils/printf.h"
#include "system/kheap.h"
#include "system/fat32.h"
#include "system/pm.h"
#include "system/proc.h"
#include "system/mmu.h"
#include "system/syscall.h"
#include "utils/elf.h"

int main() {

    serial_init();
    init_printf(NULL, serial_putc);
    printf("\nprintf initialized\n");
    kheap_init();
    printf("kernel heap initialized\n");
    vfs_init();
    printf("vfs initialized\n");
    pm_init();
    printf("phys memory initialized\n");
    vm_init();
    printf("virt memory initialized\n");

    const char path[] = "/PRG";
    process_t proc;
    proc_new(path, &proc);
    proc_switch(&proc);
    asm("eret");

    printf("looping forever...\n");
    for(;;);
    return 0;
}