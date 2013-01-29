#ifndef TIMER_H
#define TIMER_H
void timer_init(void);
unsigned int uptime(void);
void uptime_to_string(unsigned int uptime, char* string);
void print_time_diff(unsigned int device, unsigned int current, unsigned int last);

#endif
