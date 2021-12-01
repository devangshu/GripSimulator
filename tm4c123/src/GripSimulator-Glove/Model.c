// Model.c

// valvanoware
#include "CortexM.h"
#include "LaunchPad.h"
#include "PLL.h"
#include "tm4c123gh6pm.h"
// driver modules
#include "ESPDriver.h"
#include "TimerDriver.h"
//#include "ADCDriver.h"

#include "Debug.h"
#include "Model.h"

// model main method
int main_model(void) {
	PLL_Init(Bus80MHz);
	//TExaS_SetTask(&LogicAnalyzerTask);
	//TExaS_SetTask(&ScopeTask);  // analog scope PD3
	LaunchPad_Init();  // activate port F
	DisableInterrupts();

	PF2 = 0;
	Timer_Init();
	//ADC_Init(2000000, &ESP_Send_Data, 1);
	ESP_Init(800000, 1);
	EnableInterrupts();

	while (1) {
		PF1 ^= 0x02;
	}
}
