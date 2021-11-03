#include "tm4c123gh6pm.h"
#include "CortexM.h"
//#include "TExaS.h"

#include "Debug.h"

// debugging

void LogicAnalyzerTask(void) {
  UART0_DR_R = 0x80|GPIO_PORTF_DATA_R; // sends at 10kHz
}

// measures analog voltage on PD3
void ScopeTask(void) {  // called 10k/sec
  UART0_DR_R = (ADC1_SSFIFO3_R>>4); // send ADC to TExaSdisplay
}
