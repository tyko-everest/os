#include "print.h"

void print_init() {
    fb_init();
    serial_init(SERIAL_COM1_BASE);
}

int print(char* buf, unsigned int len, io_output_mode_t mode) {
    if (mode == IO_OUTPUT_FB) {
        fb_write(buf, len);
        return 0;
    } else if (mode == IO_OUTPUT_SERIAL) {
        serial_write(buf, len);
        return 0;
    } else {
        return 1;
    }
}
void print_nl(void) {
    print("\n", 1, IO_OUTPUT_FB);
}

void print_uint(unsigned int num) {
    fb_write("0x", 2);
    for (int i = 7; i >= 0; i--) {
        char hex_digit = (char) (((num >> (4 * i)) & 0xF ) + 0x30U);
        switch (hex_digit) {
        case 0x3A:
            hex_digit = 'A';
            break;
        case 0x3B:
            hex_digit = 'B';
            break;
        case 0x3C:
            hex_digit = 'C';
            break;
        case 0x3D:
            hex_digit = 'D';
            break;
        case 0x3E:
            hex_digit = 'E';
            break;
        case 0x3F:
            hex_digit = 'F';
            break;
        default:
            break;
        }
        fb_write(&hex_digit, 1);
    }
    fb_write("\n", 1);
}