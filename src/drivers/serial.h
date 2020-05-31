#ifndef INCLUDE_SERIAL_H
#define INCLUDE_SERIAL_H

#include "port.h"
#include "stddef.h"

/* Base address of the serial ports */
#define SERIAL_COM1_BASE                0x3F8

/* Macros to get specific port addresses for each serial port port */
#define SERIAL_DATA_PORT(base)          (base)
#define SERIAL_FIFO_COMMAND_PORT(base)  (base + 2)
#define SERIAL_LINE_COMMAND_PORT(base)  (base + 3)
#define SERIAL_MODEM_COMMAND_PORT(base) (base + 4)
#define SERIAL_LINE_STATUS_PORT(base)   (base + 5)

/* Mode to expect the highest 8 bits first on the data port
 * then the lowest 8 bits after
 */
#define SERIAL_LINE_ENABLE_DLAB     0x80

/** serial_configure_baud_rate
 * Sets the speed of the data being sent, speed is 115200 / divisor
 * 
 * @param com       The COM port to configure
 * @param divsior   The baud rate divisor
 */
void serial_configure_baud_rate(unsigned short com, unsigned short divisor);

/** serial_configure_line
 * Configures the line of the given serial port.
 * Current config is data length of 8 bits, no pariy bits, one stop bit,
 * and break control disabled
 * 
 * @param com The serial port to configure
 */
void serial_configure_line(unsigned short com);

/** serial_configure_buffer
 * Configures the buffer of the given serial port.
 * Current config is FIFO enabled, clear both receive and transmit queues,
 * and 14 byes as size of queue
 * 
 * @param com The serial port to configure
 */
void serial_configure_buffer(unsigned short com);

/** serial_configure_modem
 * Configures the modem of the given serial port
 * Current config does not handle interrupts, enables RTS and DTS
 * 
 * @param com The serial port to configure
 */
void serial_configure_modem(unsigned short com);

/** serial_is_transmit_fifo_empty:
 * Checks whether the transmit FIFO queue is empty or not for the given COM
 * port.
 * 
 * @param  com The COM port
 * @return 0 if the transmit FIFO queue is not empty
 *         1 if the transmit FIFO queue is empty
 */
int serial_is_transmit_fifo_empty(unsigned short com);

/** serial_init
 * Runs once to setup serial port
 * 
 * @param com The COM port to setup
 */
void serial_init(unsigned short com);

/** serial_write
 * Writes text to the serial port
 * 
 * @param buf The string to be written
 * @param len The length of the string
 */
void serial_write(const char* buf);

#endif /* INCLUDE_SERIAL_H */