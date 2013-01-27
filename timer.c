#include "io.h"
#include "x86arch.h"
#include "support.h"
#include "startup.h"

static unsigned int count = 0;
void timer_interrupt_handler(int vector, int code) {
	count++;
	if (count%100 == 0) {
		dputs(DEVICE_CONSOLE, "Timer Stuff\n");
	}
	__outb( PIC_MASTER_CMD_PORT, PIC_EOI );
}

void timer_init(void) {
	dputs(DEVICE_CONSOLE, "Initializing Timer\n");
	unsigned int divisor = TIMER_FREQUENCY/33;
	__outb(TIMER_CONTROL_PORT, TIMER_0_SELECT | TIMER_USE_DECIMAL | TIMER_MODE_3 | TIMER_0_LOAD);
	__outb(TIMER_0_PORT, divisor & 0xff);
	__outb(TIMER_0_PORT, divisor >> 8);
	__install_isr(INT_VEC_TIMER, timer_interrupt_handler);
}
