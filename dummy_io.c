/**
 * Dummy IO module:
 * Does nothing for all the specified IO commnds
 *
 * Obviously only useful for debugging.
 * Author: Dan Johnson
 */

/**
 * Always gets an empty string
 */
int dummy_gets(char *buffer, unsigned int count) {
	if (count) {
		*buffer = '\0';
		return 0;
	}
	else {
		//uh-oh, no room for null
		//unspecified behavior
		return -1;
	}
}
/**
 * Never gets a character
 */
int dummy_getchar(void) {
	return 0;
}
/**
 * Ignores the given string
 */
void dummy_puts(char *str) {
}
