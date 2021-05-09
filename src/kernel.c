#include "drivers/mmio.h"
#include "drivers/serial.h"
#include "utils/printf.h"
#include "system/fat32.h"

extern void * __files_start;

int main() {
    serial_init();
    init_printf(NULL, serial_putc);
    printf("printf initialized\n");

    printf("ram fs at: 0x%X\n", &__files_start);

    fat32_init();
    printf("fat32 initialized\n");

    uint8_t buf[512];
    int res = fat32_readfile("/HELLO", 0, 16, buf);
    printf("%d chars read\n", res);
    for (int i = 0; i < res; i ++) {
        printf("0x%X\n", buf[i]);
    }
    printf("read: %s\n", buf);

    for(;;);


    // setup sd card
    // setup GPIO22-27 as ALT3, the EMMC
    mmio_write(GPFSEL2, 0x3FFFF << 6);
    // enable SD clock
    printf("starting clock\n");
    uint32_t control1 = (0xF << 16) | 0b10000101;
    mmio_write(EMMC_CONTROL1, control1);
    _delay(1000);
    printf("clock started\n");

    // set block size 512 bytes, and transfer 1 block
    mmio_write(EMMC_BLKSIZECNT, (1 << 16) | 512);
    // enable read interrupt for polling
    mmio_write(EMMC_IRPT_MASK, 1 << 5);
    // read block 0 as argument
    mmio_write(EMMC_ARG1, 0);
    _delay(1000000);
    // CMD17 (read single block)
    // normal command type, command involves data transfer, don't check anything
    // no response expected
    // single block, card to host, no second command, disable block counter
    uint32_t command = (17 << 24) | (00100 << 19) | (00 << 16) | (01000 << 1);
    // issue command
    mmio_write(EMMC_CMDTM, command);

    printf("command issued\n");
    // while(!(mmio_read(EMMC_INTERRUPT) & (1 << 5)));
    _delay(10000000);
    printf("data ready\n");

    for (int i = 0; i < 128; i++) {
        printf("0x%x\n", mmio_read(EMMC_DATA));
    }




    printf("looping forever...");
    for(;;);
    return 0;
}