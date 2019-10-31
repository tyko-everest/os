#include "keyboard.h"

unsigned char read_scan_code(void) {
    return inb(KBD_DATA_PORT);
}

char get_key(void) {
    unsigned char scan_code = read_scan_code();
    switch (scan_code) {
    case KEY_Q:
        return 'Q';
        break;

    case KEY_W:
        return 'W';
    
    
    default:
        return 0;
        break;
    }
}