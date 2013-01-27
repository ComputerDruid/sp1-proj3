#include "sio.h"
#include "timer.h"
void os_init(void) {
	/*
	** Initialize the serial port
	*/
	sio_init();
	timer_init();
	__asm("sti");
}
