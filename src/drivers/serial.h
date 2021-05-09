#ifndef INCLUDE_SERIAL_H
#define INCLUDE_SERIAL_H

#include "drivers/delay.h"
#include "drivers/mmio.h"

void serial_init();

void serial_putc(void* p, char c);

#endif