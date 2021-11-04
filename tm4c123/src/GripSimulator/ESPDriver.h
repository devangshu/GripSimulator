#ifndef _ESPDRIVER_H_
#define _ESPDRIVER_H_


#include <stdlib.h>
#include <string.h>

#include "tm4c123gh6pm.h"
#include "CortexM.h"
#include "UARTLab4.h"
#include "esp8266Lab4.h"

#include "TimerDriver.h"


void ESP_Init(uint32_t read_period, uint8_t read_enable);

void TM4C_to_ESP(uint32_t pin, uint32_t value);

void ESP_Send_Data(uint32_t pin_num, uint32_t pin_val);

void ESP_Receive_Data(uint32_t pin_num, uint32_t pin_val);


#endif
