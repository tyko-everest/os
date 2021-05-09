#include "serial.h"

void serial_init() {
    // setup gpio for uart
    mmio_write(GPFSEL1, 0b010010 << 12);
    // setup uart
    mmio_write(AUX_ENB, 1);
    mmio_write(AUX_MU_BAUD_REG, 3254);
}

void serial_putc(void* p, char c) {
    (void) p;
	mmio_write(AUX_MU_IO_REG, c);
    _delay(10000);
}
