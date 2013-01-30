/*
** SCCS ID:	%W%	%G%
**
** File:	main.c
** Author:	K. Reek
** Contributor:	Warren Carithers
** Description:	Main program of the sample polled serial I/O program.
*/

#include "os.h"
#include "watch.h"

/*
** main
**
**	Main program
*/
int main ( void ) {
	os_init();
	normal();
	return( 0 );
}
