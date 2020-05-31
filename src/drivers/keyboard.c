#include "keyboard.h"

unsigned char read_scan_code(void) {
    return inb(KBD_DATA_PORT);
}

char get_key(void) {
    unsigned char scan_code = read_scan_code();
    char c;
    switch (scan_code) {
    case KEY_Q:
        c = 'Q';
        break;
    case KEY_W:
        c = 'W';
        break;
    case KEY_E:
        c = 'E';
        break;
    case KEY_R:
        c = 'R';
        break;
    case KEY_T:
        c = 'T';
        break;
    case KEY_Y:
        c = 'Y';
        break;
    case KEY_U:
        c = 'U';
        break;
    case KEY_I:
        c = 'I';
        break;
    case KEY_O:
        c = 'O';
        break;
    case KEY_P:
        c = 'P';
        break;
    case KEY_A:
        c = 'A';
        break;
    case KEY_S:
        c = 'S';
        break;
    case KEY_D:
        c = 'D';
        break;
    case KEY_F:
        c = 'F';
        break;
    case KEY_G:
        c = 'G';
        break;
    case KEY_H:
        c = 'H';
        break;
    case KEY_J:
        c = 'J';
        break;
    case KEY_K:
        c = 'K';
        break;
    case KEY_L:
        c = 'L';
        break;
    case KEY_Z:
        c = 'Z';
        break;
    case KEY_X:
        c = 'X';
        break;
    case KEY_C:
        c = 'C';
        break;
    case KEY_V:
        c = 'V';
        break;
    case KEY_B:
        c = 'B';
        break;
    case KEY_N:
        c = 'N';
        break;
    case KEY_M:
        c = 'M';
        break;
    case KEY_PERIOD:
        c = '.';
        break;
    case KEY_ENTER:
        c = '\n';
        break;
    case KEY_SPACE:
        c = ' ';
        break;
    default:
        c = 0;
        break;
    }
    if (c != 0) {
        kb_write_buf(c);
    }
    return c;
}

static char kb_buf[KB_BUF_SIZE];
// number of items currently in the keyboard buffer
static unsigned int kb_buf_num = 0;
// these need to be initialized to above max index
// so on first run they reset the indexes to 0 before using them
static unsigned int kb_buf_start = KB_BUF_SIZE;
static unsigned int kb_buf_end = KB_BUF_SIZE;

char kb_read_buf() {
    if (kb_buf_num == 0) {
        return 0;
    } else {
        if (kb_buf_start < KB_BUF_SIZE - 1) {
            kb_buf_start++;
        } else {
            kb_buf_start = 0;
        }
        kb_buf_num--;
        return kb_buf[kb_buf_start];
    }
}

int kb_write_buf(char c) {
    if (kb_buf_num == KB_BUF_SIZE) {
        return -1;
    } else {
        if (kb_buf_end < KB_BUF_SIZE - 1) {
            kb_buf_end++;
        } else {
            kb_buf_end = 0;
        }
        kb_buf_num++;
        kb_buf[kb_buf_end] = c;
        return 0;
    }
}
