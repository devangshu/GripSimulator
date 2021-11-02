#ifndef _TIMERDRIVER_H_
#define _TIMERDRIVER_H_

#include <stdint.h>

void Timer_Init(void);

uint32_t get_time_measurement(void);

void Timer_InitTask2(void(*task2)(void));

void Timer_InitTask3(void(*task3)(void));


#endif
