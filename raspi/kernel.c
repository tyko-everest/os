#include <stdint.h>
#include "printf.h"

#define NULL 0

typedef enum {
    PERIPH_BASE =           0x3f000000,

    GPIO_BASE =             PERIPH_BASE + 0x200000,
    GPFSEL0 =               GPIO_BASE + 0x00,
    GPFSEL1 =               GPIO_BASE + 0x04,
    GPFSEL2 =               GPIO_BASE + 0x08,
    GPSET0 =    		    GPIO_BASE + 0x1C,
    GPCLR0 =    		    GPIO_BASE + 0x28,
    GPLEV0 =    		    GPIO_BASE + 0x34,

    AUX_BASE =              PERIPH_BASE + 0x215000,
    AUX_IRQ =			    AUX_BASE + 0x00,
    AUX_ENB =               AUX_BASE + 0x04,
    AUX_MU_IO_REG =         AUX_BASE + 0x40,
    AUX_MU_BAUD_REG =       AUX_BASE + 0x68,

    EMMC_BASE =             PERIPH_BASE + 0x300000,
    EMMC_ARG2 =             EMMC_BASE + 0x00,
    EMMC_BLKSIZECNT =       EMMC_BASE + 0x04,
    EMMC_ARG1 =             EMMC_BASE + 0x08,
    EMMC_CMDTM =            EMMC_BASE + 0x0C,
    EMMC_RESP0 =            EMMC_BASE + 0x10,
    EMMC_RESP1 =            EMMC_BASE + 0x14,
    EMMC_RESP2 =            EMMC_BASE + 0x18,
    EMMC_RESP3 =            EMMC_BASE + 0x1C,
    EMMC_DATA =             EMMC_BASE + 0x20,
    EMMC_STATUS =           EMMC_BASE + 0x24,
    EMMC_CONTROL0 =         EMMC_BASE + 0x28,
    EMMC_CONTROL1 =         EMMC_BASE + 0x2C,
    EMMC_INTERRUPT =        EMMC_BASE + 0x30,
    EMMC_IRPT_MASK =        EMMC_BASE + 0x34,
    EMMC_IRPT_EN =          EMMC_BASE + 0x38,
    EMMC_CONTROL2 =         EMMC_BASE + 0x3C,
    EMMC_FORCE_IRPT =       EMMC_BASE + 0x50,
    EMMC_BOOT_TIMEOUT =     EMMC_BASE + 0x70,
    EMMC_DBG_SEL =          EMMC_BASE + 0x74,
    EMMC_EXRDFIFO_CFG =     EMMC_BASE + 0x80,
    EMMC_EXRDFIFO_EN =      EMMC_BASE + 0x84,
    EMMC_TUNE_STEP =        EMMC_BASE + 0x88,
    EMMC_TUNE_STEPS_STD =   EMMC_BASE + 0x8C,
    EMMC_TUNE_STEPS_DDR =   EMMC_BASE + 0x90,
    EMMC_SPI_INT_SPT =      EMMC_BASE + 0xF0,
    EMMC_SLOTISR_VER =      EMMC_BASE + 0xFC
} mmio_t;   

static inline uint32_t mmio_read(mmio_t reg) {
    return *((volatile uint32_t *) reg);
}

static inline void mmio_write(mmio_t reg, uint32_t val) {
    *((volatile uint32_t *) reg) = val;
}

void _delay(uint32_t count);

void putc(void* p, char c) {
	mmio_write(AUX_MU_IO_REG, c);
    _delay(10000);
}

int main() {
    // setup gpio for uart
    mmio_write(GPFSEL1, 0b010010 << 12);
    // setup uart
    mmio_write(AUX_ENB, 1);
    mmio_write(AUX_MU_BAUD_REG, 3254);

    // setup printf
    init_printf(NULL, putc);
    printf("printf initialized\n");

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