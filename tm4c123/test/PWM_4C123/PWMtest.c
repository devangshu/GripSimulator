// PWMtest.c
// Runs on TM4C123
// Use PWM0/PB6 and PWM1/PB7 to generate pulse-width modulated outputs.
// Daniel Valvano
// March 28, 2014

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
   Program 6.8, section 6.3.2

   "Embedded Systems: Real-Time Operating Systems for ARM Cortex M Microcontrollers",
   ISBN: 978-1466468863, Jonathan Valvano, copyright (c) 2015
   Program 8.4, Section 8.3

 Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
#include <stdint.h>
#include "../../inc/PLL.h"
#include "Servo.h"

void WaitForInterrupt(void);  // low power mode

int main(void){
  PLL_Init(Bus80MHz);               // bus clock at 80 MHz
  Hand_Init();

  while(1){
    //WaitForInterrupt();
	  /*
      for(int i = 0; i < 180; i++){
          Finger0_Duty(i);
          for(uint32_t j = 0; j < 50000; j++){}
      }

      for(int i = 180; i > 0; i--){
          Finger0_Duty(i);
          for(uint32_t j = 0; j < 50000; j++){}
      }
      */
	  Finger0_Duty(0);
	  for(uint32_t j = 0; j < 10000000; j++){}
	  Finger0_Duty(180);
	  for(uint32_t j = 0; j < 10000000; j++){}
  }

}
