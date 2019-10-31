#ifndef INCLUDE_PORT_H
#define INCLUDE_PORT_H

/** outb:
 * Sends the given data to the given I/O port. Defined in io.s
 * 
 * @param port The I/O port to send the data to
 * @param data The data to send to the I/O port
 */
void outb(unsigned short port, unsigned char data);

/** inb:
 * Receives data from the given I/O port. Defined in io.s
 * 
 * @param port The I/O port to receive the data from
 * @returns The received byte
 */
unsigned char inb(unsigned short port);

#endif /* INCLUDE_PORT_H */