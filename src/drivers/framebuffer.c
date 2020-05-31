#include "framebuffer.h"

// pointer to the memory-mapped framebuffer
static char *fb = (char *) FB_ADDR;
// position of the next available location to print a character
static fb_pos_t pos = {.row = 0, .col = 0};

void fb_write(const char *buf) {
    unsigned int i = 0;
    while (buf[i] != NULL) {

        if (buf[i] == '\n') {
            pos.row++;
            pos.col = 0;
        } else {
            fb_write_cell(fb_addr(pos.row, pos.col), buf[i], FB_BLACK, FB_LIGHT_GRAY);
            pos.col++;
        }

        if (pos.col == FB_WIDTH) {
            pos.col = 0;
            pos.row++;
        }
        if (pos.row == FB_HEIGHT) {
            pos.row--;
            fb_scroll_screen();
        }

        i++;
    }

    if (pos.col == FB_WIDTH - 1) {
        fb_move_cursor(fb_cursor(pos.row + 1, 0));
    } else {
        fb_move_cursor(fb_cursor(pos.row, pos.col));
    }
}

void fb_init() {
    fb_clear_screen();
}

void fb_write_cell(unsigned int i, char c, unsigned char fg, unsigned char bg) {
    fb[i] = c;
    fb[i + 1] = ((fg & 0x0F) << 4) | (bg & 0x0F);
    fb[i] = c;
}

void fb_move_cursor(unsigned short pos) {
    outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
    outb(FB_DATA_PORT, (pos >> 8) & 0x00FF);
    outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
    outb(FB_DATA_PORT, pos & 0x00FF);
}

void fb_scroll_screen() {
    for (unsigned int row = 0; row < FB_HEIGHT - 1; row++) {
        for (unsigned int col = 0; col < FB_WIDTH; col++) {
            fb[fb_addr(row, col)] = fb[fb_addr(row + 1, col)];
        }
    }
    for (unsigned int col = 0; col < FB_WIDTH; col++) {
        fb[fb_addr(FB_HEIGHT - 1, col)] = 0;
    }
}

void fb_clear_screen() {
    for (unsigned int row = 0; row < FB_HEIGHT; row++) {
        for (unsigned int col = 0; col < FB_WIDTH; col++) {
            fb[fb_addr(row, col)] = 0;
        }
    }
    fb_move_cursor(0);
}

unsigned int fb_addr(unsigned int row, unsigned int col) {
    return (row * FB_WIDTH + col) * 2;
}

unsigned int fb_cursor(unsigned int row, unsigned int col) {
    return (row * FB_WIDTH + col);
}
