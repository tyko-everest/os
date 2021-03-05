#include "stdio.h"

int putchar(int c) {
    print(&c, IO_OUTPUT_FB);
}

int puts(const char *str) {
    print(str, IO_OUTPUT_FB);
}

// this does not nearly support all printf formats at the moment
// TODO unify this and print.c
int printf(const char *format, ...) {

    int tot_chars = 0;
    va_list ap;
    va_start(ap, format);

    char *c = format;
    while (*c != NULL) {
        if (*c == '%') {
            c++;
            if (*c == '.') {
                c++;
                if (*c == '*') {
                    size_t n = va_arg(ap, size_t);
                    c++;
                    if (*c == 's') {
                        char *s = va_arg(ap, char *);
                        for (; n > 0; n--) {
                            putchar(*s);
                            tot_chars++;
                            s++;
                        }
                        c++;
                    }
                }
            } else if (*c == 'd' || *c == 'i') {

            } else if (*c == 'x' || *c == 'X') {
                uint32_t num = va_arg(ap, uint32_t);
                puts("0x");
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
                    putchar(hex_digit);
                }
                c++;
            } else if (*c == 's') {
                puts(va_arg(ap, char *));
                c++;
            }

        } else {
            putchar(*c);
            tot_chars++;
            c++;
        }
    }
    va_end(ap);
    return tot_chars;
}

int scanf(const char *format, ...) {
    va_list ap;
    va_start(ap, format);

    char *c = format;
    while (*c != NULL) {
        if (*c == '%') {
            c++;
            if (*c == 's') {
                // get a pointer to the string where this will be stored
                char *s = va_arg(ap, char *);
                unsigned int s_index = 0;
                // grab a character from the kb buffer
                char c_buf = kbd_read_buf();
                // either of these indicate the end of the string
                while (!(c_buf == 0 || c_buf == ' ' || c_buf == '\n')) {
                    s[s_index] = c_buf;
                    c_buf = kbd_read_buf();
                    s_index++;
                }
                // when finished, add the null character
                s[s_index] = 0;
                c++;
                    
            } else if (*c == 'd' || *c == 'i') {

            } else if (*c == 'X') {
            
            }

        } else {
            c++;
        }
    }
    va_end(ap);
    return 0;
}