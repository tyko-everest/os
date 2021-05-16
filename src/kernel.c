#include "arch/registers.h"
#include "clib/stdint.h"
#include "drivers/mmio.h"
#include "drivers/serial.h"
#include "utils/printf.h"
#include "system/fat32.h"
#include "system/mmu.h"

extern void * __files_start;

uint32_t _get_sctlr();

// the ram fs is currently NOT being loaded with the kernel
int main(uint64_t test) {

    // volatile double a = 1;
    // a *= 4.5;

    serial_init();
    init_printf(NULL, serial_putc);
    printf("\nprintf initialized\n");

    printf("%x passed from start.s\n", test);

    printf("looping forever...\n");
    for(;;);
    return 0;
}