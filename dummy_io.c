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
int dummy_getchar(void) {
	return 0;
}
void dummy_puts(char *str) {
}
