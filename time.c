/**
 * Implements user side time handling functions
 * Authors: Dean Knight, Dan Johnson
 */

#include "time.h"
#include "timer.h"
#include "io.h"

#define MS_PER_HOUR (1000*60*60)
#define MS_PER_MIN  (1000*60)
#define MS_PER_SEC  1000
#define MS_PER_DAY (24*MS_PER_HOUR)
#define UNCOMPRESSED_TIME_STR_SIZE 9 //Uncompressed time strings should always be nine chars
#define COMPRESSED_TIME_STR_SIZE 7   //Compressed time strings should always be seven chars

static unsigned int time_offset = 0;

/**
 * 
 * @param
 * @param
 */

/**
 * Calculate difference between two times given and print out
 * the required digits. Also takes care of backspacing flags
 * before printing
 * @param device device to print to
 * @param current current time
 * @param last last time printed to screen to be diffed against
 */
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

/**
 * Convert the specified time into a string representation.
 *
 * @param time the time to convert.
 * @param dst a string buffer of at least 9 characters to receive the string.
 */
void time_to_string(time_t time, char* dst) {
	unsigned int current_hours = (time/MS_PER_HOUR)%24;
	int current_hours_h1 = current_hours%10;
	int current_hours_h2 = current_hours/10;

	unsigned int current_minutes = (time/MS_PER_MIN)%60;
	int current_minutes_m1 = current_minutes%10;
	int current_minutes_m2 = current_minutes/10;

	unsigned int current_seconds = (time/MS_PER_SEC)%60;
	int current_seconds_s1 = current_seconds%10;
	int current_seconds_s2 = current_seconds/10;

	dst[0] = current_hours_h2 + '0';
	dst[1] = current_hours_h1 + '0';
	dst[2] = ':';

	dst[3] = current_minutes_m2 + '0';
	dst[4] = current_minutes_m1 + '0';
	dst[5] = ':';

	dst[6] = current_seconds_s2 + '0';
	dst[7] = current_seconds_s1 + '0';
	dst[8] = '\0';
}


/**
 * Extension of print_time_diff to add on the .s for timer mode
 * @param d device to print to
 * @param cur current time
 * @param last the last time to be printed on the screen
 */
void print_time_diff_ex(device_t d, unsigned int cur, unsigned int last) {
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


/**
 * prints the time
 * @param d device top print to
 * @param t time to print
 */
void print_time(device_t d, time_t t) {
	static char tmp[9];
	time_to_string(t, tmp);
	dputs(d, tmp);
}


/**
 * Extension of print_time to print the time along with the .s for timer mode
 * @param d device to print to
 * @param time time to print
 */
void print_time_ex(device_t d, unsigned int time) {
	static char tmp[9];
	time_to_string(time, tmp);
	dputs(d, tmp);
	dputchar(d, '.');
	dputchar(d, '0' + (time/100)%10);
}


/**
 * Takes an uncompressed string and compresses it by removing :'s
 * @param uncompressed pointer to uncompressed string
 * @param compressed pointer to buffer to store new compressed string
 */
void compress_time_str(char* uncompressed, char* compressed)
{
	int compressed_index = 0;
	for(int i = 0; i < UNCOMPRESSED_TIME_STR_SIZE; i++)
	{
		switch(uncompressed[i])
		{
		case ':':
			break;
		default:
			compressed[compressed_index] = uncompressed[i];
			compressed_index++;
			break;
		}
	}
}


/**
 * takes a compressed or uncompressed time string and returns the time 
 * @param time_str the string to be converted to a time in miliseconds
 * @return time in miliseconds
 */
unsigned int string_to_time(char* time_str)
{
	char compressed_time_str[7];
	unsigned int time_in_ms = 0;
	
	//if we have uncompressed time, compress it.
	//else copy it into our local variable
	if(sizeof(time_str) > 7)
	{
		compress_time_str(time_str, compressed_time_str);
	}
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

/**
 * Simple function that takes a new time (in miliseconds) and sets the 
 * global time offset 
 * @param new_time time to be set
 */
void set_time(unsigned int new_time) {
	time_offset = ((new_time%MS_PER_DAY) - (uptime()%MS_PER_DAY) + MS_PER_DAY)%MS_PER_DAY;
}


/**
 * @return returns the time in miliseconds
 */
time_t get_time(void) {
	return (time_offset + uptime()) % MS_PER_DAY;
}


/**
 * Calculates next uptime at a certain time
 * @param time time to calculate uptime at
 */
unsigned int next_uptime_at(time_t time) {
	unsigned int up = uptime();
	unsigned int current_time = (time_offset + up) % MS_PER_DAY;
	return ((time - current_time)%MS_PER_DAY + MS_PER_DAY) % MS_PER_DAY +
		up;
}
