/* ADC Driver Module */

#include "ADCDriver.h"


/* constants */

#define PF2 (*((volatile uint32_t *)0x40025010))

#define VP_NUM 0


/* module fields */

uint32_t ADC_Value;
void (*ADC_Task)(uint32_t,uint32_t);
int ADC_Trigger_Change;


/* module internal functions */

uint32_t ADC_Read(void) {
    return ADC0_InSeq3();
}

void ADC_Handler(void) {
    uint32_t new_value = ADC_Read();
    if (new_value != ADC_Value || ADC_Trigger_Change == 0) {
        ADC_Value = new_value;
        ADC_Task(VP_NUM, ADC_Value);
    }
}


/* module external methods */

void ADC_Init(uint32_t read_period, void(*task)(uint32_t,uint32_t), int trigger_on_change) {
    ADC_Value = 0;
    ADC_Task = task;
    ADC_Trigger_Change = trigger_on_change;
    //ADC0_InitSWTriggerSeq3_Ch9();  // PE4
    ADC0_InitSWTriggerSeq3_Ch7();  // PD0
    Timer_SetTask3(&ADC_Handler, read_period);
}
