#include "timer.h"
#include "io.h"
void normal(void) {
	unsigned int device = DEVICE_SERIAL;
	dputs( device, "" );

	int last = uptime();
	char time[9];
	uptime_to_string(last, time);
	dputs(device, time);
	while(1){
		int ch = dgetchar(device);
		switch(ch)
		{
			case 'a':
			dputs(device, "Switching to alarm mode\n");
			break;
			case 's':
			dputs(device, "Switching to set mode\n");
			break;
			case 't':
			dputs(device, "Switching to timer mode\n");
			break;
		}
		int cur = uptime();
		if(cur != last)
		{
			print_time_diff(device, cur, last);
			last = cur;
		}
	}
}
