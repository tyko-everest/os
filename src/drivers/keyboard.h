#ifndef INCLUDE_KEYBOARD_H
#define INCLUDE_KEYBOARD_H

#include "port.h"

typedef enum {
    KEY_ESC = 0x01,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_0,
    KEY_MINUS,
    KEY_EQUALS,
    KEY_BACKSPACE,
    KEY_TAB,
    KEY_Q,
    KEY_W,
    KEY_E,
    KEY_R,
    KEY_T,
    KEY_Y,
    KEY_U,
    KEY_I,
    KEY_O,
    KEY_P,
    KEY_LEFT_BRACKET,
    KEY_RIGHT_BRACKET,
    KEY_ENTER,
    KEY_CTRL,
    KEY_A,
    KEY_S,
    KEY_D,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_COLON,
    KEY_QUOTE,
    KEY_TILDE,
    KEY_LEFT_SHIFT,
    KEY_BACKSLASH,
    KEY_Z,
    KEY_X,
    KEY_C,
    KEY_V,
    KEY_B,
    KEY_N,
    KEY_M,
    KEY_COMMA,
    KEY_PERIOD,
    KEY_FORWARDSLASH,
    KEY_RIGHT_SHIFT,
    KEY_PRINT_SCREEN,
    KEY_ALT,
    KEY_SPACE,
    KEY_CAPS_LOCK,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6
} scan_code_t;

#define KBD_DATA_PORT 0x60

#define KB_BUF_SIZE 256

/** read_scan_code:
 * Reads a scan code from the keyboard
 * 
 * @return The scan code (NOT an ASCII character!)
 */
unsigned char read_scan_code(void);

// reads in a scan code from the keyboard and returns its ASCII character
char get_key(void);

char kb_read_buf(void);
int kb_write_buf(char c);

#endif // INCLUDE_KEYBOARD_H