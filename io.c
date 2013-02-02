#include "io.h"
#include "c_io.h"
#include "sio.h"
#include "dummy_io.h"

/**
 * List of devices
 */
struct character_device device_list[NUM_DEVICES] = {
	{"dummy", dummy_puts, dummy_gets, dummy_getchar},
	{"console", c_puts, c_gets, c_getchar},
	{"serial", sio_puts, sio_gets, sio_getchar}
};

/**
 * put a string onto the screen
 * @param d device to print to
 * @param str string to print
 */
void dputs(device_t d, char *str) {
	return device_list[d].puts(str);
}

/**
 * get a string from input
 * @param d device to get string from
 * @param str buffer to store string
 * @param count size of the string
 */
int dgets(device_t d, char *str, unsigned int count) {
	return device_list[d].gets(str, count);
}

/**
 * dgetchar get a character from input device
 * @param d device to grab the character from
 */
int dgetchar(device_t d) {
	return device_list[d].getchar();
}
/**
 * Print a character to a device
 * @param device to put the character to
 * @param c the character to put
 */
void dputchar(device_t d, char c) {
	char string[2] = { c, '\0'};
	return dputs(d, string);
}
