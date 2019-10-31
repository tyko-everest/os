#include "serial.h"

void serial_configure_baud_rate(unsigned short com, unsigned short divisor) {
    outb(SERIAL_LINE_COMMAND_PORT(com), SERIAL_LINE_ENABLE_DLAB);
    outb(SERIAL_DATA_PORT(com), (divisor >> 8) & 0x00FF);
    outb(SERIAL_DATA_PORT(com), divisor & 0x00FF);
}

void serial_configure_line(unsigned short com) {
    outb(SERIAL_LINE_COMMAND_PORT(com), 0x03);
}

void serial_configure_buffer(unsigned short com) {
    outb(SERIAL_FIFO_COMMAND_PORT(com), 0xC7);
}

void serial_configure_modem(unsigned short com) {
    outb(SERIAL_MODEM_COMMAND_PORT(com), 0x03);
}

int serial_is_transmit_fifo_empty(unsigned short com) {
    /* if bit 5 is high, FIFO queue is empty: 0x20 = 0010 0000 */
    return inb(SERIAL_LINE_STATUS_PORT(com)) & 0x20;
}

void serial_init(unsigned short com) {
    serial_configure_baud_rate(com, 1);
    serial_configure_line(com);
    serial_configure_buffer(com);
    serial_configure_modem(com);    
}

void serial_write(char* buf, unsigned int len) {
    unsigned int i = 0;
    while (len > 0) {
        // wait until the port is ready
        while(!serial_is_transmit_fifo_empty(SERIAL_COM1_BASE));
        outb(SERIAL_DATA_PORT(SERIAL_COM1_BASE), buf[i]);
        len--;
        i++;
    }
}