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

#define	CTRL_D	0x4		/* control-D character */

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
		char	ch;

		/*
		** Get a character and see if it was a CTRL-D
		*/
		ch = sio_getchar();
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
int sio_getchar( void ) {
	int	ch;

	/* Wait for the receiver to become ready */
	while( ( __inb( UA4_LSR ) & UA4_LSR_RXDA ) == 0 )
		;

	/* Read the character, strip the parity bit, check for control-D */
	ch = __inb( UA4_RXD ) & 0x7f;
	if( ch == CTRL_D ){
		ch = EOF;
	} else {
		/* If it is a return, substitute a newline, then echo */
		if( ch == '\r' ){
			ch = '\n';
		}
		sio_putchar( ch );
	}

	return ch;
}
