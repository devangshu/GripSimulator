#ifndef _ESPDRIVER_H_
#define _ESPDRIVER_H_

#include <stdint.h>

void ESP_Init(void);

void ESP_Receive_Data(uint32_t pin_num, uint32_t pin_val);


#endif
