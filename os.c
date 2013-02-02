/**
 * Initialize global drivers and everything necessary to be ready for user code
 * Authors: Dan Johnson, Dean Knight
 */

#include "sio.h"
#include "timer.h"
#include "x86arch.h"
#include "support.h"
#include "startup.h"

/**
 * Interrupt handler to ignore USB key unplug events.
 */
static void usb_interrupt_handler( int vector, int code ){
	__outb( PIC_MASTER_CMD_PORT, PIC_EOI );
	__outb( PIC_SLAVE_CMD_PORT, PIC_EOI );
}

/**
 * Initialize the "OS"-level drivers.
 */
void os_init(void) {
	//Initialize the serial controller
	sio_init();
	//Initialize the timer
	timer_init();
	//Ignore the USB device interrupt
	__install_isr( 0x2a, usb_interrupt_handler);
	__install_isr( 0x29, usb_interrupt_handler);
	//Ready to receive interrupts
	__asm("sti");
}
