#include "io.h"
#include "x86arch.h"
#include "support.h"
#include "startup.h"

#define MS_PER_HOUR (1000*60*60)
#define MS_PER_MIN  (1000*60)
#define MS_PER_SEC  1000

static unsigned int count = 0;
static void timer_interrupt_handler(int vector, int code) {
	count++;
	__outb( PIC_MASTER_CMD_PORT, PIC_EOI );
}

void timer_init(void) {
	dputs(DEVICE_CONSOLE, "Initializing Timer\n");
	unsigned int divisor = TIMER_FREQUENCY/100;
	__outb(TIMER_CONTROL_PORT, TIMER_0_SELECT | TIMER_MODE_3 | TIMER_0_LOAD);
	__outb(TIMER_0_PORT, divisor & 0xff);
	__outb(TIMER_0_PORT, divisor >> 8);
	__install_isr(INT_VEC_TIMER, timer_interrupt_handler);
}

unsigned int uptime(void) {
	return count * 10;
}

//Takes uncompressed time string and compresses (removes :'s) it
//Assumes two null terminated strings of size 9 and 7 respectively
void compress_time_str(char* uncompressed, char* compressed)
{
	int compressed_index = 0;
	for(int i = 0; i < sizeof(uncompressed); i++)
	{
		switch(uncompressed[i])
		{
		case ':':
			break;
		default:
			compressed[compressed_index] = uncompressed[i];
			compressed_index++;
		}
	}
}

//Takes a compressed or uncompressed time string and returns the time
//it represents in miliseconds
unsigned int string_to_time(char* time_str)
{
	char compressed_time_str[7];
	unsigned int time_in_ms = 0;
	
	//if we have uncompressed time, compress it.
	//else copy it into our local variable
	if(sizeof(time_str) > 7)
	compress_time_str(time_str, compressed_time_str);
	else
	{
		for(int i = 0; i < sizeof(compressed_time_str); i++)
		{
			compressed_time_str[i] = time_str[i];
		}
	}

	time_in_ms += (compressed_time_str[0] - '0')*(MS_PER_HOUR*10);
	time_in_ms += (compressed_time_str[1] - '0')*(MS_PER_HOUR);
	time_in_ms += (compressed_time_str[2] - '0')*(MS_PER_MIN*10);
	time_in_ms += (compressed_time_str[3] - '0')*(MS_PER_MIN);
	time_in_ms += (compressed_time_str[4] - '0')*(MS_PER_SEC*10);
	time_in_ms += (compressed_time_str[5] - '0')*(MS_PER_SEC);

	return time_in_ms;
}


//Simple function that takes a new count (in miliseconds) and sets the
//global count variable which is in hundredths of a second.
void set_timer_count(unsigned int new_count)
{
	count = new_count/10;
}

/******************************************************************
Name: print_time_diff
Purpose: Calculate difference between two times given and print out
	the required digits.
Params:	unsigned int current (time in ms)
	unsigned int last    (time in ms)

Return: Number of characters that differ including the : characters
	when applicable.
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
}


void uptime_to_string(unsigned int uptime, char* time)
{
	unsigned int current_hours = (uptime/MS_PER_HOUR)%24;
	int current_hours_h1 = current_hours%10;
	int current_hours_h2 = current_hours/10;


	unsigned int current_minutes = (uptime/MS_PER_MIN)%60;
	int current_minutes_m1 = current_minutes%10;
	int current_minutes_m2 = current_minutes/10;

	unsigned int current_seconds = (uptime/MS_PER_SEC)%60;
	int current_seconds_s1 = current_seconds%10;
	int current_seconds_s2 = current_seconds/10;

	time[0] = current_hours_h2 + '0';
	time[1] = current_hours_h1 + '0';
	time[2] = ':';

	time[3] = current_minutes_m2 + '0';
	time[4] = current_minutes_m1 + '0';
	time[5] = ':';

	time[6] = current_seconds_s2 + '0';
	time[7] = current_seconds_s1 + '0';
	time[8] = '\0';
}
