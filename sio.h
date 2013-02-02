/*
** SCCS ID:	%W%	%G%
**
** File:	sio.h
** Author:	K. Reek
** Contributor:	Warren Carithers
** Description:	Prototypes for serial I/O routines
*/

#ifndef _SIO_H_
#define _SIO_H_

#define	EOF	-1

void sio_init( void );
int sio_gets( char *buffer, unsigned int bufsize );
void sio_puts( char *buffer );
void sio_putchar( char ch );
int sio_getchar( void );

#endif
