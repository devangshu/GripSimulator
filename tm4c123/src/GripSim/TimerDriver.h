#ifndef _TIMERDRIVER_H_
#define _TIMERDRIVER_H_


#include <stdio.h>
#include <stdint.h>
#include "tm4c123gh6pm.h"

#include "Timer2.h"
#include "Timer3.h"


void Timer_Init();

uint32_t get_time_measurement(void);

void Timer_SetTask2(void(*task2)(void), uint32_t period);

void Timer_SetTask3(void(*task3)(void), uint32_t period);


#endif
