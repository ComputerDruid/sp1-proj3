#ifndef TIME_H
#define TIME_H

#include "io.h" //for device_t
void print_time_diff_ex(device_t d, unsigned int cur, unsigned int last);
void print_time_ex(device_t d, unsigned int time);
void print_time_diff(unsigned int device, unsigned int current, unsigned int last);
void uptime_to_string(unsigned int uptime, char* string);
void set_timer_count(unsigned int new_count);
void compress_time_str(char* uncompressed, char* compressed);
int string_to_time(char* time_string);
#endif
