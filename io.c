#include "io.h"
#include "c_io.h"
#include "sio.h"
#include "dummy_io.h"

struct character_device device_list[NUM_DEVICES] = {
	{"dummy", dummy_puts, dummy_gets, dummy_getchar},
	{"console", c_puts, c_gets, c_getchar},
	{"serial", sio_puts, sio_gets, sio_getchar}
};

void dputs(unsigned int device, char *str) {
	return device_list[device].puts(str);
}

int dgets(unsigned int device, char *str, unsigned int count) {
	return device_list[device].gets(str, count);
}

int dgetchar(unsigned int device) {
	return device_list[device].getchar();
}

void dputchar(unsigned int device, char c) {
	char string[2] = { c, '\0'};
	return dputs(device, string);
}
