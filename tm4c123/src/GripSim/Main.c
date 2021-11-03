// Main.c

#include <stdint.h>
#include <stdio.h>

#include "tm4c123gh6pm.h"
#include "PLL.h"
#include "LaunchPad.h"
#include "CortexM.h"
//#include "TExaS.h"

// driver modules
#include "TimerDriver.h"
#include "ESPDriver.h"
#include "ADCDriver.h"


// debugging
#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))
void LogicAnalyzerTask(void) {
  UART0_DR_R = 0x80|GPIO_PORTF_DATA_R; // sends at 10kHz
}
// measures analog voltage on PD3
void ScopeTask(void) {  // called 10k/sec
  UART0_DR_R = (ADC1_SSFIFO3_R>>4); // send ADC to TExaSdisplay
}


int main(void) {

  PLL_Init(Bus80MHz);
  //TExaS_SetTask(&LogicAnalyzerTask);
  //TExaS_SetTask(&ScopeTask);  // analog scope PD3
  LaunchPad_Init();  // activate port F
  DisableInterrupts();

  PF2 = 0;
  Timer_Init();
  ADC_Init(400000000, &ESP_Send_Data, 1);
  ESP_Init(800000);

  EnableInterrupts();

  while (1) {
    PF1 ^= 0x02;
  }

}
