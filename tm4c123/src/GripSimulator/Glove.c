// Glove.c

// valvanoware
#include "CortexM.h"
#include "LaunchPad.h"
#include "PLL.h"
#include "tm4c123gh6pm.h"
// driver modules
#include "ADCDriver.h"
#include "Debug.h"
#include "ESPDriver.h"
#include "Glove.h"
#include "TimerDriver.h"

// glove main method
int main_glove(void) {
	PLL_Init(Bus80MHz);
	//TExaS_SetTask(&LogicAnalyzerTask);
	//TExaS_SetTask(&ScopeTask);  // analog scope PD3
	LaunchPad_Init();  // activate port F
	DisableInterrupts();

	PF2 = 0;
	Timer_Init();
	ADC_Init(2000000, &ESP_Send_Data, 1);
	ESP_Init(800000, 0);
	EnableInterrupts();

	while (1) {
		PF1 ^= 0x02;
	}
}
