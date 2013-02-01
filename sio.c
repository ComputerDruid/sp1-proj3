/*
** SCCS ID:	%W%	%G%
**
** File:	sio.c
** Author:	K. Reek
** Contributor:	Warren Carithers
** Description:	Polled serial I/O routines
*/

#include "sio.h"
#include "startup.h"
#include <uart.h>
#include "x86arch.h"
#include "support.h"
#include "sleep.h"
#define	CTRL_D	0x4		/* control-D character */

static int current_char = 0;
#define WRITE_BUF_LEN 1024
char write_buf[WRITE_BUF_LEN];
int write_buf_start = 0;
int write_buf_end = 0;

//NOTE: iterrupts must not be enabled when this routine is run
//This routine must only be called when the device is ready to accept output
static void write_one_byte(void) {
	//ensure write_buf_end is consistent.
	write_buf_end %= WRITE_BUF_LEN;

	if (write_buf_start == write_buf_end) return;

	char ch = write_buf[write_buf_start];

	/* If this character is a newline, send out a return first */
	/* FIXME: this \r handling code is fragile and doesn't work for all cases */
	if( ch == '\n' ){
		ch = '\r';
		write_buf[write_buf_start] = '\r';
		write_buf_start--;
	}
	else if (ch == '\r') {
		ch = '\n';
		write_buf[write_buf_start] = '\n';
	}
	__outb( UA4_TXD, ch );
	write_buf_start = (write_buf_start + 1)%WRITE_BUF_LEN;
}

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
/*
** sio_init
**
**	Initialize the serial port chip
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

static void write_start(void) {
	while (__inb( UA4_LSR ) & UA4_LSR_TXRDY) {
		if (write_buf_start == write_buf_end) return;
		__asm("cli");
		if (__inb( UA4_LSR ) & UA4_LSR_TXRDY) {
			write_one_byte();
		}
		__asm("sti");
	}
	//Dropped out of the loop; the device is no longer ready for input
	//When it becomes ready, it will interrupt to let us know
}

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

/*
** sio_puts
**
**	Write the null-terminated string to the serial port.
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

/*
** sio_gets
**
**	Read a line into the user's buffer, echoing all characters.
**	The line is terminated with either a return or a newline.
**
**	CTRL-D (which is returned to us as EOF) causes sio_gets to
**	return to the program right away without storing anything
**	else in the user's buffer.
**
**	Returns the number of characters in the line before the NUL.
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

/*
** sio_putchar
**
**	Write a single character to the device
*/
void sio_putchar( char ch ) {
	static char buf[2] = {'\0', '\0'};
	buf[0] = ch;
	sio_puts(buf);
}

/*
** sio_getchar
**
**	Read a single character from the device
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
