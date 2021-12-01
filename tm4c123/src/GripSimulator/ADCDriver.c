/* ADC Driver Module */

#include "ADCDriver.h"

/* constants */

#define PF2 (*((volatile uint32_t *)0x40025010))

#define VP_NUM 0

/* module fields */

uint32_t ADC_Value;
void (*ADC_Task)(uint32_t, uint32_t);
int ADC_Trigger_Change;

/* module internal functions */

uint32_t ADC_Read_Angle(void) {
	uint32_t step = 2;	// 5
	uint32_t adc_in_val = ADC0_InSeq3();
	uint32_t value = ADC0_InSeq3() / (4096 / 100);
	value /= step;
	return step * value;
}

void ADC_Handler(void) {
    uint32_t old_adc_value = ADC_Value;
	uint32_t new_adc_value = ADC_Read_Angle();
	uint32_t adc_val_avg = 0;
	if (new_adc_value != ADC_Value || ADC_Trigger_Change == 0) {
		ADC_Value = new_adc_value;
		adc_val_avg = (old_adc_value + new_adc_value) / 2;
		(*ADC_Task)(VP_NUM, adc_val_avg);
	}
}

/* module external methods */

void ADC_Init(uint32_t read_period, void (*task)(uint32_t, uint32_t), int trigger_on_change) {
	ADC_Value = 0;
	ADC_Task = task;
	ADC_Trigger_Change = trigger_on_change;
	// ADC0_InitSWTriggerSeq3_Ch9();  // PE4
	ADC0_InitSWTriggerSeq3_Ch7();  // PD0
	Timer_SetTask2(&ADC_Handler, read_period);
}
