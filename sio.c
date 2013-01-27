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

	__outb(UA4_IER, UA4_IER_RX_INT_ENABLE);
}

/*
** sio_puts
**
**	Write the null-terminated string to the serial port.
*/
void sio_puts( char *buffer ) {
	char	ch;

	/*
	** Get characters one by one and write them to the port
	*/
	while( (ch = *buffer++) != '\0' ) {
		sio_putchar( ch );
	}
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

	/* If this character is a newline, send out a return first */
	if( ch == '\n' ){
		sio_putchar( '\r' );
	}

	/* Wait for the transmitter to become ready */
	while( ( __inb( UA4_LSR ) & UA4_LSR_TXRDY ) == 0 )
		;
	__outb( UA4_TXD, ch );
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
		/* If it is a return, substitute a newline, then echo */
		if( ch == '\r' ){
			ch = '\n';
		}
		sio_putchar( ch );
	}
	if(ch)
	current_char = 0;

	return ch;
}
