#include "arch/registers.h"
#include "clib/stdint.h"
#include "drivers/mmio.h"
#include "drivers/serial.h"
#include "utils/printf.h"
#include "system/fat32.h"
#include "system/mmu.h"

extern void * __files_start;

uint32_t _get_sctlr();

int syscall_test(uint64_t a, uint64_t b, uint64_t c) {
    asm("svc 0");
    return -1;
}

// the ram fs is currently NOT being loaded with the kernel
int main(uint64_t test) {

    volatile int a = 1;

    serial_init();
    init_printf(NULL, serial_putc);
    printf("\nprintf initialized\n");

    printf("making syscall\n");
    syscall_test(10, 14, -5);
    printf("back from syscall\n");

    printf("looping forever...\n");
    for(;;);
    return 0;
}