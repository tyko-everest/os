#ifndef INCLUDE_IO_H
#define INCLUDE_IO_H

#include "serial.h"
#include "framebuffer.h"

typedef enum {
    IO_OUTPUT_FB = 0,
    IO_OUTPUT_SERIAL
} io_output_mode_t;

void print_init(void);

int print(const char* buf, io_output_mode_t mode);

#endif /* INCLUDE_IO_H */