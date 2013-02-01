#include "io.h"
#include "support.h"
static void tfault(void) {
	__asm("lidt 0");
	__asm("int3");
}
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
