/*
** SCCS ID:	%W%	%G%
**
** File:	main.c
** Author:	K. Reek
** Contributor:	Warren Carithers
** Description:	Main program of the sample polled serial I/O program.
*/

#include "sio.h"
#include "io.h"

#define	BUFSIZE	100

/*
** print_integer
**
**	Print an integer value on the serial I/O port
*/
void print_integer( unsigned int device, int value ) {
	int	quotient = value / 10;

	if( quotient > 0 ){
		print_integer( device, quotient );
	}
	dputchar( device, value % 10 + '0' );
}

/*
** main
**
**	Main program
*/
int main ( void ) {
	char	buffer[ BUFSIZE ];
	int	len;
	unsigned int device = DEVICE_SERIAL;

	/*
	** Initialize the serial port
	*/
	sio_init();

	/*
	** Prompt for input, get a reply, tell them how many characters
	** there were, and repeat until end of file is reached.
	*/
	dputs( device, "\n? " );
	while( (len = dgets( device, buffer, sizeof( buffer ) )) > 0 ){
		dputs( device, "You entered: " );
		dputs( device, buffer );
		dputs( device, "which is " );
		print_integer( device, len );
		dputs( device, " characters.\n? " );
	}
	dputs(device, "No More Input.\n");

	return( 0 );
}
