/**
 * Defines the user side code of our time handling methods
 * Authors: Dean Knight, Dan Johnson
 */
#ifndef TIME_H
#define TIME_H

#include "io.h" //for device_t
typedef unsigned int time_t;

void print_time_diff_ex(device_t d, unsigned int cur, unsigned int last);
void print_time_ex(device_t d, unsigned int time);
void print_time_diff(unsigned int device, unsigned int current, unsigned int last);
void print_time(device_t d, time_t t);
void time_to_string(time_t time, char* dst);
void compress_time_str(char* uncompressed, char* compressed);
unsigned int string_to_time(char* time_string);
void set_time(time_t new_time);
time_t get_time(void);
unsigned int next_uptime_at(time_t time);
#endif
