/**
 * Driver to interface with the serial controller/console with interrupts.
 * Authors: Dan Johnson, Dean Knight
 * Loosely based on: sio_polled by K. Reek and Warren Carithers.
 */

#include "sio.h"
#include "startup.h"
#include <uart.h>
#include "x86arch.h"
#include "support.h"
#include "sleep.h"
#define	CTRL_D	0x4

static int current_char = 0;
#define WRITE_BUF_LEN 1024
char write_buf[WRITE_BUF_LEN];
int write_buf_start = 0;
int write_buf_end = 0;

/**
 * Writes a byte of the buffer to the serial console.
 * NOTE: CPU interrups must be disabled when this routine is called.
 */
static void write_one_byte(void) {
	static int extra_cr_done = 0;
	//ensure write_buf_end is consistent.
	write_buf_end %= WRITE_BUF_LEN;

	if (write_buf_start == write_buf_end) return;

	char ch = write_buf[write_buf_start];

	/* If this character is a newline, send out a return first */
	if( ch == '\n') {
		if (extra_cr_done) {
			extra_cr_done = 0;
		}
		else {
			ch = '\r';
			extra_cr_done = 1;
			write_buf_start--;
		}
	}
	__outb( UA4_TXD, ch );
	write_buf_start = (write_buf_start + 1)%WRITE_BUF_LEN;
}

/**
 * Interrupt handler for serial controller interrupts.
 */
static void io_interrupt_handler(int vector, int code) {
	int eventID = __inb(UA4_EIR) & UA4_EIR_INT_PRI_MASK;
	while(eventID != UA4_EIR_NO_INT)
	{
		//and the eventID to turn reserved/FIFO bits to 0
		switch(eventID)
		{
			case UA4_EIR_LINE_STATUS:
			__inb(UA4_LSR);
			break;

			case UA4_EIR_RX_HIGH:
			case UA5_EIR_RX_FIFO_TO:
			current_char = __inb(UA4_RXD) & 0x7f;
			break;

			case UA4_EIR_TX_LOW:
			write_one_byte();
			break;

			case UA4_EIR_MODEM_STATUS:
			__inb(UA4_MSR);
			break;
		}
		eventID = __inb(UA4_EIR) & UA4_EIR_INT_PRI_MASK;
	}
	__outb( PIC_MASTER_CMD_PORT, PIC_EOI );
}

/**
 * Initializes the serial controller.
 */
void sio_init( void ) {
	/*
	** Select bank 1 and set the data rate.
	*/
	__outb( UA4_LCR, UA4_LCR_BANK1 );
	__outb( UA4_LBGD_L, BAUD_LOW_BYTE( BAUD_9600 ) );
	__outb( UA4_LBGD_H, BAUD_HIGH_BYTE( BAUD_9600 ) );

	/*
	** Select bank 0, and at the same time set the LCR for our
	** data characteristics.
	*/
	__outb( UA4_LCR, UA4_LCR_BANK0 | UA4_LCR_BITS_8 |
	    UA4_LCR_1_STOP_BIT | UA4_LCR_NO_PARITY );
	/*
	** Set Modem Control Register to send and receive
	** This is necessary to receive data from the ADDS terminals
	*/
	__outb( UA4_MCR, UA4_MCR_DTR | UA4_MCR_RTS |
	        UA4_MCR_ISEN );

	__install_isr(INT_VEC_SERIAL_PORT_1, io_interrupt_handler);

	__outb(UA4_IER, UA4_IER_RX_INT_ENABLE | UA4_IER_TX_INT_ENABLE);
}

/**
 * Starts a write sequence, if data is not currently being written.
 */
static void write_start(void) {
	while (__inb( UA4_LSR ) & UA4_LSR_TXRDY) {
		if (write_buf_start == write_buf_end) return;
		__asm("cli");
		if (__inb( UA4_LSR ) & UA4_LSR_TXRDY) {
			write_one_byte();
		}
		__asm("sti");
		//loop if we somehow missed the TX ready interrupt while CPU
		//interrupts are disabled.
	}
	//Dropped out of the loop; the device is no longer ready for input
	//When it becomes ready, it will interrupt to let us know
}

/**
 * Asynchronously writes the specified byte sequence to the serial console.
 * @param buffer the data to write from
 * @param nbytes the number of bytes to write
 */
void sio_write(char *buffer, unsigned int nbytes) {
	//c_printf("Writing:%s:%d %d-%d\n", buffer, nbytes, write_buf_start, write_buf_end);
	int spaceleft = WRITE_BUF_LEN - ((write_buf_end - write_buf_start) %
		WRITE_BUF_LEN + WRITE_BUF_LEN) % WRITE_BUF_LEN;
	if (spaceleft < nbytes) {
		__panic("No space left in serial output buffer\n");
	}
	int index = write_buf_end;
	unsigned int count;
	for (count = 0; count < nbytes; ++count) {
		write_buf[index] = buffer[count];
		++write_buf_end;
		write_buf_end %= WRITE_BUF_LEN;
		index = (index + 1) % WRITE_BUF_LEN;
	}
	write_start();
}

/**
 * Asynchronously prints a null-terminated string to the serial console.
 */
void sio_puts( char *buffer ) {
	char *temp = buffer;
	unsigned int len = 0;

	//poor man's strlen
	while( *buffer++ != '\0' ) {
		++len;
	}

	sio_write(temp, len);
}

/**
 * Gets a NUL-terminated line of text from the serial console.
 * Ends with a '\n' if a whole line was read. If EOF (CTRL-D) is read, or the
 * buffer is filled, it does not end with a '\n'.
 *
 * NOTE: This routine blocks waiting for input. While this is arguably not
 * allowed in this project, it does not matter, because this routine is never
 * called by the watch program. It is simply included for completeness and
 * debugging.
 *
 * @param buffer to put the read string
 * @param bufsize the maximum number of chars to read, counting the ending NUL.
 * @return the number of characters read, not counting the ending NUL.
 */
int sio_gets( char *buffer, unsigned int bufsize ) {
	char	*bp = buffer;

	/*
	** Loop as long as there is space in the buffer, leaving room for
	** the terminating null byte.
	*/
	while( bufsize > 1 ) {
		char	ch = 0;

		/*
		** Get a character and see if it was a CTRL-D
		*/
		while(!(ch = sio_getchar()))
		{
			sleep();
		}
		if( ch == EOF ) {
			/* Activate without storing anything */
			break;
		}

		/* Store in the caller's buffer */
		*bp++ = ch;
		bufsize -= 1;

		/* If this is a newline, activate */
		if( ch == '\n' ) {
			break;
		}
	}

	/* Null-terminate the buffer and return */
	*bp = '\0';
	return bp - buffer;
}

/**
 * Writes a character to the serial console.
 */
void sio_putchar( char ch ) {
	static char buf[2] = {'\0', '\0'};
	buf[0] = ch;
	sio_puts(buf);
}

/**
 * gets a character from the serial console, if one is waiting.
 * @return 0 if no character is waiting, the waiting character, otherwise.
 */
int sio_getchar( void ){
	int ch = current_char;


	/* Read the character, strip the parity bit, check for control-D */
	if( ch == CTRL_D ){
		ch = EOF;
	} else {
		/* If it is a return, substitute a newline */
		if( ch == '\r' ){
			ch = '\n';
		}
	}
	if(ch)
	current_char = 0;

	return ch;
}
