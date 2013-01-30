#include "timer.h"
#include "io.h"

//global mode bits:
static int alarm_set = 0;


static int last_flag_alarm = 0;
static int last_flag_lap = 0;
static void print_flags(device_t d, int lap) {
	dputchar(d, alarm_set ? '*' : ' ');
	dputchar(d, lap ? 'L' : ' ');
	last_flag_alarm = alarm_set;
	last_flag_lap = lap;
}
static void print_flags_diff(device_t d, int lap) {
	if (last_flag_alarm != alarm_set) {
		dputs(d, "");
		print_flags(d, lap);
	}
	else if (last_flag_lap != lap) {
		dputs(d, lap ? "L" : " ");
	}
	last_flag_alarm = alarm_set;
	last_flag_lap = lap;
}

static void print_time_diff_ex(device_t d, unsigned int cur, unsigned int last) {
	if (last/1000 != cur/1000) {
		dputs(d, "");
		print_time_diff(d, cur, last);
		dputchar(d, '.');
		dputchar(d, '0' + (cur/100)%10);
	}
	else if ((last/100)%10 != (cur/100)%10) {
		dputchar(d, '');
		dputchar(d, '0' + (cur/100)%10);
	}
}

static void print_time_ex(device_t d, unsigned int time) {
	static char tmp[9];
	uptime_to_string(time, tmp);
	dputs(d, tmp);
	dputchar(d, '.');
	dputchar(d, '0' + (time/100)%10);
}

void timer(device_t d) {
	unsigned int start_time;
	static unsigned int accumulated_time = 0;
	unsigned int last = 0;
	int timer_mode = 1;
	int timer_running = 0;
	int lap_mode = 0;

	print_time_ex(d, accumulated_time);
	print_flags(d, lap_mode);
	last = accumulated_time;

	while(timer_mode){
		int ch = dgetchar(d);
		if (ch) dputs(d, " "); //compensate for typed character
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
			case 'l':
				lap_mode = !lap_mode;
			break;
			case 'r':
				dputs(d, "\nSwitching back to normal mode\n");
				timer_mode = 0;
			break;
		}
		if (!timer_mode) break; //skip display update
		unsigned int cur = accumulated_time +
		                   (timer_running ? uptime() - start_time : 0);
		if((cur/100 != last/100)&& !lap_mode)
		{
			dputs(d, ""); //clear flags
			print_time_diff_ex(d, cur, last);
			print_flags(d, lap_mode);
			last = cur;
		}
		else {
			print_flags_diff(d, lap_mode);
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
