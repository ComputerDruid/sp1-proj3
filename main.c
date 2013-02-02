/**
 * Main module which serves as the entry point for the system
 *
 * Authors: Dan Johnson, Dean Knight
 */
#include "os.h"
#include "watch.h"

/**
 * Entry point for the system.
 */
int main (void) {
	//Initialize OS
	os_init();
	//Start user program
	normal();
	return 0;
}
