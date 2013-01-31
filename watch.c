#include "timer.h"
#include "io.h"
#include "mystery.h"

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
		erase_typed_char(d, ch);
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

int set(device_t d, unsigned int cur_time)
{
	char cur_time_str[9];
	char cur_time_str_compressed[7];
	for(int k = 0; k < sizeof(cur_time_str_compressed); k++)
	{
		cur_time_str_compressed[k] = '0';
	}
	cur_time_str_compressed[6] = '\0';
	//Get cur_time as a string, compress it for use later
	uptime_to_string(cur_time, cur_time_str);
	compress_time_str(cur_time_str, cur_time_str_compressed);

	//Clear the screen, print cur_time_str
	dputchar(d, ''); 
	dputs(d, cur_time_str);
	int setting_time = 1;

	//Move the cursor back eight spaces to allow user to set the time
	for(int i = 0; i < 8; i++)
	{
		dputchar(d, '');
	}
	char ch = 0;
	
	char digits[7];
	for(int i = 0; i < sizeof(digits); i++)
	{
		digits[i] = ' ';
	}
	digits[6] = '\0';
	int digit_index = 0;

	//Set the digits of the clock
	while(setting_time)
	{
		ch = dgetchar(d);
		if((ch >= '0' && ch <= '9') || ch == ' ')
		{
			if(ch == ' ')
			{
				dputchar(d, '');
				dputchar(d, cur_time_str_compressed[digit_index]);
				digit_index++;
			}
			else
			{
				digits[digit_index] = ch;
				digit_index++;
			}
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
			default:
				erase_typed_char(d, ch);
			}
		}
	}

	//Get rid of spaces in digits[] and put in digits from cur_time_compressed_str[]
	for(int i = 0; i < sizeof(digits); i++)
	{
		if(digits[i] == ' ')
		{
			digits[i] = cur_time_str_compressed[i];
		}
	}
	/*
	dputs(DEVICE_CONSOLE, digits);
	dputs(DEVICE_CONSOLE, "\n");
	dputs(DEVICE_CONSOLE, cur_time_str_compressed);
	dputs(DEVICE_CONSOLE, "\n");
	dputs(DEVICE_CONSOLE, cur_time_str);
	dputs(DEVICE_CONSOLE, "\n");
	DEBUG CAN BE STRIPPED*/
	return string_to_time(digits);
}


static char mystery_code[] = { 0xb, 0xb, 0x16, 0x16, 0x8, 0xc, 0x8, 0xc, 0x62, 0x0 };
void normal(void) {
	unsigned int device = DEVICE_SERIAL;
	dputs( device, "" );

	int mystery_pos = 0;
	int last = uptime();
	char time[9];
	uptime_to_string(last, time);
	dputs(device, time);
	int cur = 0;
	while(1){
		int ch = dgetchar(device);
		if (ch) {
			erase_typed_char(device, ch);
			switch(ch)
			{
				case 'a':
				if (mystery_code[mystery_pos])
					dputs(device, "Switching to alarm mode\n");
				else mystery(device);
				break;
				case 's':
				dputs(device, "Switching to set mode\n");
				last = set(device, cur);
				set_timer_count(last);
				uptime_to_string(last, time);
				dputchar(device, '');
				dputs(device, time);
				break;
				case 't':
				dputs(device, "\nSwitching to timer mode\n");
				timer(device);
				uptime_to_string(last, time);
				dputs(device, time);
				break;
			}
			if (ch == mystery_code[mystery_pos]) {
				mystery_pos++;
			}
			else {
				mystery_pos = 0;
			}
		}
		cur = uptime();
		if(cur != last)
		{
			print_time_diff(device, cur, last);
			last = cur;
		}
	}
}
