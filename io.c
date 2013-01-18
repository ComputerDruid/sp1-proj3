#include "io.h"
#include "c_io.h"
#include "sio.h"
#include "dummy_io.h"

struct character_device device_list[NUM_DEVICES] = {
	{"dummy", dummy_puts, dummy_gets},
	{"console", c_puts, c_gets},
	{"serial", sio_puts, sio_gets}
};

void dputs(unsigned int device, char *str) {
	return device_list[device].puts(str);
}

int dgets(unsigned int device, char *str, unsigned int count) {
	return device_list[device].gets(str, count);
}

void dputchar(unsigned int device, char c) {
	char string[2] = { c, '\0'};
	return dputs(device, string);
}
