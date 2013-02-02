#include "io.h"
#include "x86arch.h"
#include "support.h"
#include "startup.h"

static unsigned int count = 0;

/**
 * Interrupt handler for timer ticks.
 */
static void timer_interrupt_handler(int vector, int code) {
	count++;
	__outb( PIC_MASTER_CMD_PORT, PIC_EOI );
}

/**
 * Initialize the timer.
 * Sets the timer to 100Hz and installs our interrupt handler.
 */
void timer_init(void) {
	dputs(DEVICE_CONSOLE, "Initializing Timer\n");
	unsigned int divisor = TIMER_FREQUENCY/100;
	__outb(TIMER_CONTROL_PORT, TIMER_0_SELECT | TIMER_MODE_3 | TIMER_0_LOAD);
	__outb(TIMER_0_PORT, divisor & 0xff);
	__outb(TIMER_0_PORT, divisor >> 8);
	__install_isr(INT_VEC_TIMER, timer_interrupt_handler);
}

/**
 * @return the number of milliseconds the system has been running.
 */
unsigned int uptime(void) {
	return count * 10;
}
