#ifndef _ADCDRIVER_H_
#define _ADCDRIVER_H_


#include <stdint.h>
#include <stdio.h>

#include "tm4c123gh6pm.h"
#include "ADCSWTrigger.h"


void ADC_Init(uint32_t read_period, void(*task)(uint32_t,uint32_t), int trigger_on_change);

uint32_t ADC_Read(void);

void ADC_Handler(void);


#endif
