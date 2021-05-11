#ifndef INCLUDE_MMIO_H
#define INCLUDE_MMIO_H

#include "clib/stdint.h"
#include "drivers/delay.h"

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

uint32_t mmio_read(mmio_t reg);
void mmio_write(mmio_t reg, uint32_t val);


#endif
