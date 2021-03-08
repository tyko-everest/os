/**
 * The keyboard driver
 *  
 * Works with the standard PC keyboard scanscode set 2
 * For now only generates a stream of characters, does not keep
 * information regarding key unpresses
 * 
 * Current status:
 * - Supports main rectangle keys (except: caps lock, meta, print screen)
 * 
 * Known bugs:
 * - if both shift keys are pressed, and only one is depressed,
 *   the driver will generate lower case
 */

#ifndef INCLUDE_KEYBOARD_H
#define INCLUDE_KEYBOARD_H

#include "port.h"
#include "clib/stdio.h"

#define KBD_CODE_CTRL   0x1D
#define KBD_CODE_SHIFT  0x2A
#define KBD_CODE_RSHIFT 0x36
#define KBD_CODE_ALT    0x38
#define KBD_CODE_SPACE  0x39
// note this is scancode esc, not ascii escape char
#define KBD_CODE_ESC    0xE0

// FORNOW: only supports scan codes from space and below
#define KBD_CODE_LAST   KBD_CODE_SPACE 

#define KBD_MOD_SHIFT   (1 << 0)
#define KBD_MOD_CTRL    (1 << 1)
#define KBD_MOD_ALT     (1 << 2)

#define KBD_DATA_PORT 0x60

#define KB_BUF_SIZE 256


/** keyboard_int:
 * Interface with the interrupt system, lets the driver know a new scan code
 * is available
 * 
 */
void keyboard_int(void);

char kbd_read_buf(void);
static int kbd_write_buf(char c);

#endif // INCLUDE_KEYBOARD_H
