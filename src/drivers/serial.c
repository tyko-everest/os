#include "serial.h"

void serial_init() {
    // setup pins 14 and 15 as alt5, the miniuart
    mmio_write(GPFSEL1, 0b010010 << 12);
    // setup pins 14 and 15 as floating
    mmio_write(GPPUD, 0);
    _delay(150);
    mmio_write(GPPUDCLK0, (1 << 14) | (1 << 15));
    _delay(150);
    mmio_write(GPPUD, 0);
    mmio_write(GPPUDCLK0, 0);
    // setup uart
    mmio_write(AUX_ENB, 1);
    // mmio_write(AUX_MU_CNTL_REG, 0);
    // mmio_write(AUX_MU_IER_REG,0);
    // mmio_write(AUX_MU_LCR_REG, 0b11);
    // mmio_write(AUX_MU_MCR_REG, 0);
    mmio_write(AUX_MU_BAUD_REG, 270);
    // mmio_write(AUX_MU_CNTL_REG, 0b11);

    // clear the receive buffer before interrupts are enabled
    mmio_write(AUX_MU_IIR_REG, 1 << 1);
    // setup interrupts for serial receive
    // THE DATASHEET IS WRONG, it has transmit and recieve IRQs bits flipped
    mmio_write(AUX_MU_IER_REG, 1 << 0);
    mmio_write(ENABLE_IRQS1, 1 << 29);
}

void serial_putc(void* p, char c) {
    (void) p;
    while (mmio_read(AUX_MU_STAT_REG) & (1 << 5));
	mmio_write(AUX_MU_IO_REG, c);
}
