#ifndef _ADCDRIVER_H_
#define _ADCDRIVER_H_


#include <stdint.h>

#include "tm4c123gh6pm.h"
#include "ADCSWTrigger.h"
#include "CortexM.h"

#include "TimerDriver.h"

void ADC_Init(uint32_t read_period, void(*task)(uint32_t,uint32_t), int trigger_on_change);

void ADC_Read(void);

void ADC_Handler(void);

void ADC_Init_5Chan(void);

void ADC_In5(uint32_t data[5]);

#endif
