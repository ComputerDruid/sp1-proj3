#include "timer.h"
#include "io.h"
#include "mystery.h"
#include "time.h"

//global mode bits:
static int alarm_set = 0;
static int alarm_rang = 0;

static int alarm_time = 0;
static int last_flag_alarm = 0;
static int last_flag_lap = 0;
static void check_alarm(device_t d, unsigned int cur_time)
{
	if(alarm_set && !alarm_rang && (cur_time > alarm_time))
	{
		dputchar(d, 0x07);
		alarm_rang = 1;
	}
}

static void clear_flags(device_t d) {
	dputs(d, "");
}
static void print_flags(device_t d, int lap) {
	dputs(d, alarm_set ? " *" : "  ");
	dputs(d, lap ? " L" : "  ");
	last_flag_alarm = alarm_set;
	last_flag_lap = lap;
}

static void print_flags_diff(device_t d, int lap) {
	if (last_flag_alarm != alarm_set) {
		clear_flags(d);
		print_flags(d, lap);
	}
	else if (last_flag_lap != lap) {
		dputs(d, lap ? "L" : " ");
	}
	last_flag_alarm = alarm_set;
	last_flag_lap = lap;
}

static void erase_typed_char(device_t d, int ch) {
	if (ch) {
		switch (ch) {
			case 0x16:
				dputchar(d, 0xb);
				break;
			case 0xb:
				dputchar(d, 0x16);
				break;
			case 0x8:
				dputchar(d, 0xc);
				break;
			case 0xc:
				dputchar(d, 0x8);
				break;
			default:
				dputs(d, " "); //compensate for typed character
		}
	}
}
void watch_display_ex_new(device_t d, time_t t, int lap) {
	print_time_ex(d, t);
	print_flags(d, lap);
}
void watch_display_ex(device_t d, time_t new, time_t old, int lap) {
	if (old/100 != new/100) {
		clear_flags(d);
		print_time_diff_ex(d, new, old);
		print_flags(d, lap);
	}
	else {
		print_flags_diff(d, lap);
	}

}
void watch_display_new(device_t d, time_t t, int lap) {
	print_time(d, t);
	dputs(d, "  "); //add extra spaces
	print_flags(d, lap);
}
void watch_display(device_t d, time_t new, time_t old, int lap) {
	if (old/1000 != new/1000) {
		clear_flags(d);
		dputs(d, ""); //clear extra spaces
		print_time_diff(d, new, old);
		dputs(d, "  "); //add extra spaces
		print_flags(d, lap);
	}
	else {
		print_flags_diff(d, lap);
	}
}

void timer(device_t d) {
	unsigned int start_time;
	static unsigned int accumulated_time = 0;
	unsigned int last = 0;
	int timer_mode = 1;
	int timer_running = 0;
	int lap_mode = 0;

	watch_display_ex_new(d, accumulated_time, lap_mode);
	last = accumulated_time;

	while(timer_mode){
		int ch = dgetchar(d);
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
				timer_mode = 0;
			break;
		}
		if (!timer_mode) break; //skip display update
		unsigned int cur = accumulated_time +
		                   (timer_running ? uptime() - start_time : 0);
		if (lap_mode) {
			watch_display_ex(d, last, last, lap_mode);
		}
		else {
			watch_display_ex(d, cur, last, lap_mode);
			last = cur;
		}
	}
	//save accumulated time
	accumulated_time += timer_running? uptime() - start_time : 0;
	//return cursor to default position
	dputs(d, "\r");
}

void alarm(device_t d)
{
	//Alarm_set bit is immediately set
	alarm_set = 1;

	//Needed Variables
	char alarm_time_str[9];
	char digits[7];
	digits[6] = '\0';

	//Get alarm_time as a string, compress it for use later
	uptime_to_string(alarm_time, alarm_time_str);
	compress_time_str(alarm_time_str, digits);

	//print alarm_time_str
	dputs(d, alarm_time_str);
	int setting_time = 1;

	//Move the cursor back eight spaces to allow user to set the time
	for(int i = 0; i < 8; i++)
	{
		dputchar(d, '');
	}
	char ch = 0;

	int digit_index = 0;

	//Set the digits of the clock
	while(setting_time)
	{
		ch = dgetchar(d);
		if((ch >= '0' && ch <= '9') || ch == ' ')
		{
			if (ch == ' ')
				ch = digits[digit_index];
			else
				digits[digit_index] = ch;
			dputchar(d, ch);
			digit_index++;
			if(digit_index == 2 || digit_index == 4)
			dputchar(d, ':');
			else if(digit_index == 6)
			{
				setting_time = 0; //All done
			}
		}
		else
		{
			switch(ch)
			{
			case 'r':
				setting_time = 0; //All done
			}
		}
	}

	dputchar(d, '\r');
	//Set the alarm time after conversion
	alarm_time = string_to_time(digits);
}

int set(device_t d, unsigned int cur_time)
{
	char cur_time_str[9];
	char digits[7];
	digits[6] = '\0';
	//Get cur_time as a string, compress it for use later
	uptime_to_string(cur_time, cur_time_str);
	compress_time_str(cur_time_str, digits);

	dputs(d, cur_time_str);
	int setting_time = 1;

	//Move the cursor back eight spaces to allow user to set the time
	for(int i = 0; i < 8; i++)
	{
		dputchar(d, '');
	}
	char ch = 0;

	int digit_index = 0;

	//Set the digits of the clock
	while(setting_time)
	{
		ch = dgetchar(d);
		if((ch >= '0' && ch <= '9') || ch == ' ')
		{
			if(ch == ' ')
				ch = digits[digit_index];
			else
				digits[digit_index] = ch;
			dputchar(d, ch);
			digit_index++;
			if(digit_index == 2 || digit_index == 4)
			dputchar(d, ':');
			else if(digit_index == 6)
			{
				setting_time = 0; //All done
			}
		}
		else
		{
			switch(ch)
			{
			case 'r':
				setting_time = 0; //All done
			}
		}
	}
	dputs(d, "\r");
	return string_to_time(digits);
}

static char mystery_code[] = { 0xb, 0xb, 0x16, 0x16, 0x8, 0xc, 0x8, 0xc, 0x62, 0x0 };
void normal(void) {
	unsigned int device = DEVICE_SERIAL;
	device_t d = device;
	dputs( device, "" );

	int mystery_pos = 0;
	int last = get_time();
	int cur = last;
	watch_display_new(d, cur, 0);
	while(1){
		int ch = dgetchar(device);
		if (ch) {
			switch(ch)
			{
				case 'a':
				if (mystery_code[mystery_pos])
				{
					dputchar(device, '\r');
					alarm(device);
					watch_display_new(d, cur, 0);
				}
				else mystery(device);
				break;
				case 's':
				dputchar(device, '\r');
				last = set(device, cur);
				set_time(last);
				watch_display_new(d, last, 0);
				break;
				case 't':
				dputchar(device, '\r');
				timer(device);
				watch_display_new(d, cur, 0);
				break;
			}
			if (ch == mystery_code[mystery_pos]) {
				mystery_pos++;
			}
			else {
				mystery_pos = 0;
			}
		}
		cur = get_time();
		watch_display(d, cur, last, 0);
		if(cur/1000 != last/1000)
		{
			check_alarm(device, cur);
		}
		last = cur;
	}
}

