/*
** SCCS ID:	%W%	%G%
**
** File:	main.c
** Author:	K. Reek
** Contributor:	Warren Carithers
** Description:	Main program of the sample polled serial I/O program.
*/

#include "os.h"
#include "io.h"
#include "timer.h"
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
	unsigned int device = DEVICE_SERIAL;

	os_init();

	dputs( device, "" );

	int last = uptime();
	char time[9];
	uptime_to_string(last, time);
	dputs(device, time);
	while(1){
		int ch = dgetchar(device);
		switch(ch)
		{
			case 'a':
			dputs(device, "Switching to alarm mode\n");
			break;
			case 's':
			dputs(device, "Switching to set mode\n");
			break;
			case 't':
			dputs(device, "Switching to timer mode\n");
			break;
		}
		int cur = uptime();
		if(cur != last)
		{
			print_time_diff(device, cur, last);
			last = cur;
		}
	}
	dputs(device, "No More Input.\n");

	return( 0 );
}
