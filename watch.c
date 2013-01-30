#include "timer.h"
#include "io.h"
void timer(int device) {
	unsigned int start_time;
	static unsigned int accumulated_time = 0;
	unsigned int last = 0;
	int timer_mode = 1;
	int timer_running = 0;

	char time[9];
	uptime_to_string(0, time);
	dputs(device, time);

	while(timer_mode){
		int ch = dgetchar(device);
		if (ch) dputs(device, " "); //compensate for typed character
		switch(ch)
		{
			case 's':
				if (timer_running)
					accumulated_time += uptime() - start_time;
				else
					start_time = uptime();
				timer_running = !timer_running;
			break;
			case 'c':
				start_time = uptime();
				accumulated_time = 0;
			break;
			case 'r':
				dputs(device, "\nSwitching back to normal mode\n");
				timer_mode = 0;
			break;
		}
		unsigned int cur = accumulated_time +
		                   (timer_running ? uptime() - start_time : 0);
		if(cur != last)
		{
			print_time_diff(device, cur, last);
			last = cur;
		}
	}
	//save accumulated time
	accumulated_time += timer_running? uptime() - start_time : 0;
}

void normal(void) {
	unsigned int device = DEVICE_SERIAL;
	dputs( device, "" );

	int last = uptime();
	char time[9];
	uptime_to_string(last, time);
	dputs(device, time);
	while(1){
		int ch = dgetchar(device);
		if (ch) dputs(device, " "); //compensate for typed character
		switch(ch)
		{
			case 'a':
			dputs(device, "Switching to alarm mode\n");
			break;
			case 's':
			dputs(device, "Switching to set mode\n");
			break;
			case 't':
			dputs(device, "\nSwitching to timer mode\n");
			timer(device);
			uptime_to_string(last, time);
			dputs(device, time);
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
