#include "print.h"

void print_init() {
    fb_init();
    serial_init(SERIAL_COM1_BASE);
}

int print(char* buf, io_output_mode_t mode) {
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
void print_nl(void) {
    print("\n", IO_OUTPUT_FB);
}

void print_uint(unsigned int num) {
    fb_write("0x");
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
        // need to make a null terminated string before printing
        char hex_string[2];
        hex_string[0] = hex_digit;
        hex_string[1] = NULL;
        fb_write(&hex_string);
    }
    fb_write("\n");
}