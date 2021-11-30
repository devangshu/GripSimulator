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
	uint32_t step = 5;	// 1
	uint32_t value = ADC0_InSeq3() / (4096 / 100);
	value /= step;
	return step * value;
}

void ADC_Handler(void) {
	uint32_t new_adc_value = ADC_Read_Angle();
	if (new_adc_value != ADC_Value || ADC_Trigger_Change == 0) {
		ADC_Value = new_adc_value;
		(*ADC_Task)(VP_NUM, ADC_Value);
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

/*
 *
 * UNTESTED
 *
 */
void ADC_Init_5Chan(void){
  volatile uint32_t delay;
  SYSCTL_RCGCADC_R |= 0x00000001; // 1) activate ADC0
  SYSCTL_RCGCGPIO_R |= 0x10;      // 1) activate clock for Port E
  while((SYSCTL_PRGPIO_R&0x10) == 0){};
  while((SYSCTL_PRADC_R&0x0001) != 0x0001){};    // good code, but not yet implemented in simulator

  // Setting up GPIO pins for ADC input
  GPIO_PORTE_DIR_R &= ~0x1F;      // 3) make PE4-0 input
  GPIO_PORTE_AFSEL_R |= 0x1F;     // 4) enable alternate function on PE4-0
  GPIO_PORTE_DEN_R &= ~0x1F;      // 5) disable digital I/O on PE4-0
  GPIO_PORTE_PCTL_R = GPIO_PORTE_PCTL_R&0xFFF00000;
  GPIO_PORTE_AMSEL_R |= 0x1F;     // 6) enable analog functionality on PE4-0

  ADC0_PC_R &= ~0xF;              // 8) clear max sample rate field
  ADC0_PC_R |= 0x1;               //    configure for 125K samples/sec
  ADC0_SSPRI_R = 0x3210;          // 9) Sequencer 3 is lowest priority

  ADC0_ACTSS_R &= ~0x0004;        // 10) disable sample sequencer 2
  ADC0_EMUX_R &= ~0x0F00;         // 11) seq2 is software trigger
  ADC0_SAC_R = 0x06;              // 32-point average
  ADC0_SSMUX2_R = 0x0123;         // 12) set channels for SS2
  ADC0_SSCTL2_R = 0x6000;         // 13) no TS0 D0 IE0 END0 TS1 D1, yes IE1 END1

  ADC0_ACTSS_R &= ~0x0008;        // 9) disable sample sequencer 3
  ADC0_EMUX_R &= ~0xF000;         // 10) seq3 is software trigger
  ADC0_SSMUX3_R &= ~0x000F;       // 11) clear SS3 field
  ADC0_SSMUX3_R += 9;             //    set channel
  ADC0_SSCTL3_R = 0x0006;         // 12) no TS0 D0, yes IE0 END0
  ADC0_IM_R &= ~0x0008;           // 13) disable SS3 interrupts
  ADC0_ACTSS_R |= 0x0008;         // 14) enable sample sequencer 3

  ADC0_IM_R &= ~0x0004;           // 14) disable SS2 interrupts
  ADC0_ACTSS_R |= 0x0004;         // 15) enable sample sequencer 2
}

void ADC_In5(uint32_t data[5]){
  ADC0_PSSI_R = 0x0004;            // 1) initiate SS2
  while((ADC0_RIS_R&0x04)==0){};   // 2) wait for conversion done
  data[3] = ADC0_SSFIFO2_R&0xFFF;  // 3A) PE3 result
  data[2] = ADC0_SSFIFO2_R&0xFFF;  // 3A) PE2 result
  data[1] = ADC0_SSFIFO2_R&0xFFF;  // 3A) PE1 result
  data[0] = ADC0_SSFIFO2_R&0xFFF;  // 3B) PE0 result
  ADC0_ISC_R = 0x0004;             // 4) acknowledge completion

  ADC0_PSSI_R = 0x0008;            // 1) initiate SS3
  while((ADC0_RIS_R&0x08)==0){};   // 2) wait for conversion done
  data[4] = ADC0_SSFIFO3_R&0xFFF;  // 3) PE4 result
  ADC0_ISC_R = 0x0008;             // 4) acknowledge completion
}

