#include "arch/registers.h"
#include "clib/stdint.h"
#include "drivers/mmio.h"
#include "drivers/serial.h"
#include "utils/printf.h"
#include "system/fat32.h"
#include "system/mmu.h"
#include "system/syscall.h"

extern void * __files_start;

uint32_t _get_sctlr();

ssize_t read(const char *path, void *buf, size_t count, size_t from) {
    asm("svc 0");
}

int main() {

    serial_init();
    init_printf(NULL, serial_putc);
    printf("\nprintf initialized\n");

    fat32_init();
    const char *path = "/HELLO";
    char buf[100];

    printf("making read\n");
    volatile int res = read(path, buf, 100, 0);
    printf("read from file: %s\n", buf);

    printf("looping forever...\n");
    for(;;);
    return 0;
}