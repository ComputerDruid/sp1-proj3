/**
 * Defines IO operations within this OS
 * Authors: Dean Knight, Dan Johnson
 */

#ifndef IO_H
#define IO_H
struct character_device {
	char *name;
	void (*puts)(char*);
	int (*gets)(char*, unsigned int count);
	int (*getchar)(void);
};

extern struct character_device device_list[];
#define NUM_DEVICES 3
#define DEVICE_DUMMY 0
#define DEVICE_CONSOLE 1
#define DEVICE_SERIAL 2

typedef unsigned int device_t;
int dgets(device_t d, char *str, unsigned int count);
void dputs(device_t d, char *str);
void dputchar(device_t d, char c);
int dgetchar(device_t d);
#endif
