#ifndef INCLUDE_FRAMEBUFFER_H
#define INCLUDE_FRAMEBUFFER_H

#include "port.h"
#include "stddef.h"

#define FB_BLACK            0
#define FB_BLUE             1   
#define FB_GREEN            2
#define FB_CYAN             3
#define FB_RED              4
#define FB_MAGENTA          5
#define FB_BROWN            6
#define FB_LIGHT_GRAY       7
#define FB_DARK_GREY        8
#define FB_LIGHT_BLUE       9
#define FB_LIGHT_GREEN      10
#define FB_LIGHT_CYAN       11
#define FB_LIGHT_RED        12 
#define FB_LIGHT_MAGENTA    13
#define FB_LIGHT_BROWN      14
#define FB_WHITE            15

// loaded at same addr plus + 3 GiB due to higher half kernel
#define FB_ADDR 0xC00B8000

#define FB_WIDTH 80
#define FB_HEIGHT 25

/* The I/O ports */
#define FB_COMMAND_PORT         0x3D4
#define FB_DATA_PORT            0x3D5

/* The I/O port commands */
#define FB_HIGH_BYTE_COMMAND    14
#define FB_LOW_BYTE_COMMAND     15

typedef struct {
    unsigned int row;
    unsigned int col;
} fb_pos_t;

/** Used to write to the framebuffer in a continuous stream
 * 
 * @param buf The string to be displayed
 */
void fb_write(char *buf);

/** fb_init
 * Called before using framebuffer, clear the screen currently
 */
void fb_init(void);

/** fb_write_cell
 * Writes a character with the given foreground and background to position i
 * in the framebuffer
 * 
 * @param i  The location in the framebuffer
 * @param c  The character
 * @param fg The foreground colour
 * @param bg The background colour
 */
void fb_write_cell(unsigned int i, char c, unsigned char fg, unsigned char bg);

/** fb_move_cursor
 * Moves the cursor of the framebuffer to the given position
 * 
 * @param pos The new position of the cursor
 */
void fb_move_cursor(unsigned short pos);

/** fb_scroll_screen
 * Scrolls the screen down one row
 */
void fb_scroll_screen(void);

/** fb_clear_screen
 * Clears the screen
 */
void fb_clear_screen(void);

/**
 * Helper function to get address of specific row-col location
 * 
 * @param row The row of the location
 * @param col The col of the location
 * @returns The address from the base of the framebuffer pointer
 */
unsigned int fb_addr(unsigned int row, unsigned int col);

/**
 * Helper function to get cursor location
 * 
 * @param row The row of the location
 * @param col The col of the location
 * @returns The cursor location
 */
unsigned int fb_cursor(unsigned int row, unsigned int col);

#endif /* INCLUDE_FRAMEBUFFER_H */