#include "io.h"
#include "c_io.h"
#include "sio.h"
#include "dummy_io.h"

struct character_device device_list[NUM_DEVICES] = {
	{"dummy", dummy_puts, dummy_gets, dummy_getchar},
	{"console", c_puts, c_gets, c_getchar},
	{"serial", sio_puts, sio_gets, sio_getchar}
};

void dputs(device_t d, char *str) {
	return device_list[d].puts(str);
}

int dgets(device_t d, char *str, unsigned int count) {
	return device_list[d].gets(str, count);
}

int dgetchar(device_t d) {
	return device_list[d].getchar();
}

void dputchar(device_t d, char c) {
	char string[2] = { c, '\0'};
	return dputs(d, string);
}
