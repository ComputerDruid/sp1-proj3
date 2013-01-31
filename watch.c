#include "timer.h"
#include "io.h"
#include "mystery.h"

#define MS_PER_HOUR (1000*60*60)
#define MS_PER_MIN  (1000*60)
#define MS_PER_SEC  1000
//global mode bits:
static int alarm_set = 0;
static int alarm_rang = 0;

static int alarm_time = 0;
static int last_flag_alarm = 0;
static int last_flag_lap = 0;
static void print_flags(device_t d, int lap) {
	dputchar(d, alarm_set ? '*' : ' ');
	dputchar(d, lap ? 'L' : ' ');
	last_flag_alarm = alarm_set;
	last_flag_lap = lap;
}

static void check_alarm(device_t d, unsigned int cur_time)
{
	if(alarm_set && !alarm_rang && (cur_time > alarm_time))
	{
		dputchar(d, 0x07);
		alarm_rang = 1;
	}
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

void alarm(device_t d)
{
	//Alarm_set bit is immediately set
	alarm_set = 1;

	//Needed Variables
	char alarm_time_str[9];
	char alarm_time_str_compressed[7];
	for(int k = 0; k < sizeof(alarm_time_str_compressed); k++)
	{
		alarm_time_str_compressed[k] = '0';
	}
	alarm_time_str_compressed[6] = '\0';

	//Get alarm_time as a string, compress it for use later
	uptime_to_string(alarm_time, alarm_time_str);
	compress_time_str(alarm_time_str, alarm_time_str_compressed);

	//Clear the screen, print alarm_time_str
	dputchar(d, '');
	dputs(d, alarm_time_str);
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
				dputchar(d, alarm_time_str_compressed[digit_index]);
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

	//Get rid of spaces in digits[] and put in digits from alarm_time_compressed_str[]
	for(int i = 0; i < sizeof(digits); i++)
	{
		if(digits[i] == ' ')
		{
			digits[i] = alarm_time_str_compressed[i];
		}
	}

	//Set the alarm time after conversion
	alarm_time = string_to_time(digits);
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
	dputs(d, "\nSwitching back to normal mode\n");
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
	print_flags(device, 0);
	int cur = 0;
	while(1){
		int ch = dgetchar(device);
		if (ch) {
			erase_typed_char(device, ch);
			switch(ch)
			{
				case 'a':
				if (mystery_code[mystery_pos])
				{
					alarm(device);
					dputchar(device,'');
					uptime_to_string(last, time);
					dputs(device, time);
					print_flags(device, 0);
				}
				else mystery(device);
				break;
				case 's':
				dputs(device, "\nSwitching to set mode\n");
				last = set(device, cur);
				set_timer_count(last);
				uptime_to_string(last, time);
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
			check_alarm(device, cur);
			last = cur;
		}
	}
}


/******************************************************************
Name: print_time_diff
Purpose: Calculate difference between two times given and print out
	the required digits. Also takes care of backspacing flags
	before printing
Params:	unsigned int current (time in ms)
	unsigned int last    (time in ms)
******************************************************************/
void print_time_diff(unsigned int device, unsigned int current, unsigned int last)
{
	//This is the digit scheme used in variable names below:
	//	h2h1:m2m1:s2s1

	int diff = 0;

	//Calculate current digits because they are always possibly needed
	unsigned int current_minutes = (current/MS_PER_MIN)%60;
	int current_minutes_m1 = current_minutes%10;
	int current_minutes_m2 = current_minutes/10;

	unsigned int current_seconds = (current/MS_PER_SEC)%60;
	int current_seconds_s1 = current_seconds%10;
	int current_seconds_s2 = current_seconds/10;

	unsigned int current_hours =   (current/MS_PER_HOUR)%24;
	int current_hours_h1 = current_hours%10;
	int current_hours_h2 = current_hours/10;

	//Calculate Hours Digits

	unsigned int last_hours =   (last/MS_PER_HOUR)%24;
	int last_hours_h1 = last_hours%10;
	int last_hours_h2 = last_hours/10;

	//short circuit if different
	if(current_hours_h2 - last_hours_h2)
		diff = 8;
	else if(current_hours_h1 - last_hours_h1)
		diff = 7;

	if(!diff)
	{
		//Calculate Minutes digits

		unsigned int last_minutes = (last/MS_PER_MIN)%60;
		int last_minutes_m1 = last_minutes%10;
		int last_minutes_m2 = last_minutes/10;



		//short circuit if different
		if(current_minutes_m2 - last_minutes_m2)
			diff = 5;
		else if(current_minutes_m1 - last_minutes_m1)
			diff = 4;
	}

	if(!diff)
	{
		//Calculate Seconds digits
		unsigned int last_seconds = (last/MS_PER_SEC)%60;
		int last_seconds_s1 = last_seconds%10;
		int last_seconds_s2 = last_seconds/10;


		//short circuit if different
		if(current_seconds_s2 - last_seconds_s2)
			diff = 2;
		else if(current_seconds_s1 - last_seconds_s1)
			diff = 1;
	}

	//Backspace flags if a difference is found
	if(diff)
	{
		dputs(device, "");
	}

	//Diff now has the proper value
	for(int i = 0; i < diff; i++)
	{
		dputchar(device, '');
	}
	switch(diff)
	{
		case 8:
		dputchar(device, (char)(current_hours_h2 + '0'));
		case 7:
		dputchar(device, (char)(current_hours_h1 + '0'));
		case 6:
		dputchar(device, ':');
		case 5:
		dputchar(device, (char)(current_minutes_m2 + '0'));
		case 4:
		dputchar(device, (char)(current_minutes_m1 + '0'));
		case 3:
		dputchar(device, ':');
		case 2:
		dputchar(device, (char)(current_seconds_s2 + '0'));
		case 1:
		dputchar(device, (char)(current_seconds_s1 + '0'));
	}
	//Reprint flags after time has printed
	if(diff)
	{
		print_flags(device, 0);
	}
}
