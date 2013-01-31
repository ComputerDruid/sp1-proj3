#include "io.h"
#include "x86arch.h"
#include "support.h"
#include "startup.h"

#define MS_PER_HOUR (1000*60*60)
#define MS_PER_MIN  (1000*60)
#define MS_PER_SEC  1000
#define UNCOMPRESSED_TIME_STR_SIZE 9 //Uncompressed time strings should always be nine chars
#define COMPRESSED_TIME_STR_SIZE 7   //Compressed time strings should always be seven chars
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

//Takes a compressed or uncompressed time string and returns the time
//it represents in miliseconds
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


//Simple function that takes a new count (in miliseconds) and sets the
//global count variable which is in hundredths of a second.
void set_timer_count(unsigned int new_count)
{
	count = new_count/10;
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
