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
    fat32_init();
    printf("fat32 initialized\n");
    pm_init();
    printf("phys memory initialized\n");
    vm_init();
    printf("virt memory initialized\n");

    const char path[] = "/PRG";
    process_t proc;
    proc_load(path, &proc);
    proc_start(&proc);

    printf("looping forever...\n");
    for(;;);
    return 0;
}