#include "sio.h"
#include "timer.h"
#include "x86arch.h"
#include "support.h"
#include "startup.h"
static void usb_interrupt_handler( int vector, int code ){
	__outb( PIC_MASTER_CMD_PORT, PIC_EOI );
	__outb( PIC_SLAVE_CMD_PORT, PIC_EOI );
}
void os_init(void) {
	/*
	** Initialize the serial port
	*/
	sio_init();
	timer_init();
	//Ignore the USB device interrupt
	__install_isr( 0x2a, usb_interrupt_handler);
	__install_isr( 0x29, usb_interrupt_handler);
	__asm("sti");
}
