#include "print.h"

void print_init() {
    fb_init();
    serial_init(SERIAL_COM1_BASE);
}

int print(const char* buf, io_output_mode_t mode) {
    if (mode == IO_OUTPUT_FB) {
        fb_write(buf);
        return 0;
    } else if (mode == IO_OUTPUT_SERIAL) {
        serial_write(buf);
        return 0;
    } else {
        return 1;
    }
}
