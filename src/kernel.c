#include "drivers/mmio.h"
#include "drivers/serial.h"
#include "utils/printf.h"
#include "system/fat32.h"
#include "system/mmu.h"

extern void * __files_start;

uint32_t _get_sctlr();

// the ram fs is currently NOT being loaded with the kernel
int main() {
    serial_init();
    init_printf(NULL, serial_putc);
    printf("\nprintf initialized\n");

    uint64_t reg = read_sctlr_el1();
    printf("0x%X\n", reg);

    printf("looping forever...\n");
    for(;;);
    return 0;
}