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

int dgets(unsigned int device, char *str, unsigned int count);
void dputs(unsigned int device, char *str);
void dputchar(unsigned int device, char c);
#endif
