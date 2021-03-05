/**
 * The keyboard driver
 *  
 * Works with the standard PC keyboard scanscode set 2
 * For now only generates a stream of characters, does not keep
 * information regarding key unpresses
 */

#ifndef INCLUDE_KEYBOARD_H
#define INCLUDE_KEYBOARD_H

#include "port.h"
#include "clib/stdio.h"

#define KBD_CODE_CTRL   0x1D
#define KBD_CODE_SHIFT  0x2A
#define KBD_CODE_ALT    0x38
#define KBD_CODE_SPACE  0x39
// note this is scancode esc, not ascii escape char
#define KBD_CODE_ESC    0xE0

// FORNOW: only supports scan codes from space and below
#define KBD_CODE_LAST   KBD_CODE_SPACE 

enum kbd_mod {
    KBD_MOD_NONE,
    KBD_MOD_CTRL,
    KBD_MOD_SHIFT,
    KBD_MOD_ALT
};

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
    KEY_BACKTICK,
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
    KEY_CAPS_LOCK
} scan_code_t;

#define KBD_DATA_PORT 0x60

#define KB_BUF_SIZE 256


/** keyboard_int:
 * Interface with the interrupt system, lets the driver know a new scan code
 * is available
 * 
 */
void keyboard_int(void);

// reads in a scan code from the keyboard and returns its ASCII character
char get_key(void);

char kbd_read_buf(void);
int kbd_write_buf(char c);

#endif // INCLUDE_KEYBOARD_H