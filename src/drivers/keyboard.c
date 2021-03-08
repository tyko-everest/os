#include "keyboard.h"

static const char base_map[KBD_CODE_LAST + 1] = {
      -1,0x1b, '1', '2', '3', '4', '5', '6', // 00 - 07
     '7', '8', '9', '0', '-', '=','\b','\t', // 08 - 0F
     'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', // 10 - 17
     'o', 'p', '[', ']','\n',  -1, 'a', 's', // 18 - 1F
     'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', // 20 - 27
    '\'', '`',  -1,'\\', 'z', 'x', 'c', 'v', // 28 - 2F
     'b', 'n', 'm', ',', '.', '/',  -1,  -1, // 30 - 37
      -1, ' '
};

static const char shift_map[KBD_CODE_LAST + 1] = {
      -1,0x1b, '!', '@', '#', '$', '%', '^', // 00 - 07
     '&', '*', '(', ')', '_', '+','\b','\t', // 08 - 0F
     'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', // 10 - 17
     'O', 'P', '{', '}','\n',  -1, 'A', 'S', // 18 - 1F
     'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', // 20 - 27
    '\"', '~',  -1, '|', 'Z', 'X', 'C', 'V', // 28 - 2F
     'B', 'N', 'M', '<', '>', '?',  -1,  -1, // 30 - 37
      -1, ' '
};

unsigned int state = 0;

void keyboard_int(void) {

    unsigned char scan_code = inb(KBD_DATA_PORT);
    
    switch (scan_code) {
    case KBD_CODE_ESC:
        break;

    case KBD_CODE_CTRL:
        state |= KBD_MOD_CTRL;
        break;

    case KBD_CODE_SHIFT:
    case KBD_CODE_RSHIFT:
        state |= KBD_MOD_SHIFT;
        break;
    
    case KBD_CODE_ALT:
        state |= KBD_MOD_ALT;
        break;

    case KBD_CODE_CTRL | 0x80:
        state &= ~KBD_MOD_CTRL;
        break;
    
    case KBD_CODE_SHIFT | 0x80:
    case KBD_CODE_RSHIFT | 0x80:
        state &= ~KBD_MOD_SHIFT;
        break;

    case KBD_CODE_ALT | 0x80:
        state &= ~KBD_MOD_ALT;
        break;

    default:
        // doesn't support scan codes above this number
        if (scan_code > KBD_CODE_LAST) {
            return;
        }

        char c = -1;
        if (state & KBD_MOD_SHIFT) {
            c = shift_map[scan_code];
        } else {
            c = base_map[scan_code];
        }

        if (c != -1) {
            if (state & KBD_MOD_CTRL) {
                // allow both capitals and lower case to make the control codes
                if (c >= 'a' && c <= 'z') {
                    c -= 'a' - 'A';
                }
                // if ends up in 3rd col of ascii table after this, generate code
                // if not ignore it, invalid combo
                if (c >= '@' && c <= '_') {
                    c -= '@';
                } else {
                    return;
                }
                // now we know a valid control character will be output
            }
            // if alt is held down, just need to add an ESC character before
            if (state & KBD_MOD_ALT) {
                kbd_write_buf('\e');
            }

            kbd_write_buf(c);
        }
        break;
    }

    char test = kbd_read_buf();
    printf("Char(s): ");
    while (test != 0) {
        printf("%c, ", test);
        test = kbd_read_buf();
    }
    printf("\n");

}



static char kb_buf[KB_BUF_SIZE];
// number of items currently in the keyboard buffer
static unsigned int kb_buf_num = 0;
// these need to be initialized to above max index
// so on first run they reset the indexes to 0 before using them
static unsigned int kb_buf_start = KB_BUF_SIZE;
static unsigned int kb_buf_end = KB_BUF_SIZE;

char kbd_read_buf() {
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

static int kbd_write_buf(char c) {
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
