/**
 * External interface for the dummy_io module. Used to add methods to table in
 * io module.
 */
#ifndef DUMMY_IO_H
#define DUMMY_IO_H
int dummy_gets(char *str, unsigned int count);
int dummy_getchar(void);
void dummy_puts(char *str);
#endif
