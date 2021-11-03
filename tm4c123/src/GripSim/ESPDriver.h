#ifndef _ESPDRIVER_H_
#define _ESPDRIVER_H_


#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "tm4c123gh6pm.h"
#include "TimerDriver.h"
#include "UARTLab4.h"
#include "esp8266Lab4.h"


void ESP_Init(uint32_t read_period);

void ESP_Send_Data(uint32_t pin_num, uint32_t pin_val);

void ESP_Receive_Data(uint32_t pin_num, uint32_t pin_val);


#endif
