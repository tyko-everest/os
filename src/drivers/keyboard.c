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
        break;
    case KEY_E:
        return 'E';
        break;
    case KEY_R:
        return 'R';
        break;
    case KEY_T:
        return 'T';
        break;
    case KEY_U:
        return 'U';
        break;
    case KEY_I:
        return 'I';
        break;
    case KEY_O:
        return 'O';
        break;
    case KEY_P:
        return 'P';
        break;
    case KEY_A:
        return 'A';
        break;
    case KEY_S:
        return 'S';
        break;
    case KEY_D:
        return 'D';
        break;
    case KEY_F:
        return 'F';
        break;
    case KEY_G:
        return 'G';
        break;
    case KEY_H:
        return 'H';
        break;
    case KEY_J:
        return 'J';
        break;
    case KEY_K:
        return 'K';
        break;
    case KEY_L:
        return 'L';
        break;
    case KEY_Z:
        return 'Z';
        break;
    case KEY_X:
        return 'X';
        break;
    case KEY_C:
        return 'C';
        break;
    case KEY_V:
        return 'V';
        break;
    case KEY_B:
        return 'B';
        break;
    case KEY_N:
        return 'N';
        break;
    case KEY_M:
        return 'M';
        break;
    case KEY_ENTER:
        return '\n';
        break;
    case KEY_SPACE:
        return ' ';
        break;

    default:
        return 0;
        break;
    }
}