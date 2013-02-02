/**
 * mystery menu written just for fun.
 * Author: Dan Johnson
 */

#include "io.h"
#include "support.h"

/**
 * Triple fault and reset the machine
 */
static void tfault(void) {
	//load invalid IDT
	__asm("lidt 0");
	//cause an interrupt
	__asm("int3");
}

/**
 * Secret menu only accessible via special character combination.
 * Allows the user to reset the machine or leave the menu.
 * @param d the device to convert into a menu.
 */
void mystery(device_t d) {
	dputs(d, "Hah!\n");
	dputs(d, "s - System RESET : r - return to normal mode\n");
	int looping = 1;
	while(looping) {
		int ch = dgetchar(d);
		switch(ch) {
			case 's':
				dputs(d, "Rebooting\n");
				tfault();
			break;
			case 'r':
				dputs(d, "\nSwitching back to normal mode\n");
				looping = 0;
			break;
		}
	}
}
