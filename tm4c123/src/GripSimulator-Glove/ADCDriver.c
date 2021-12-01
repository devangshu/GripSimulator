/* ADC Driver Module */

#include "ADCDriver.h"

/* constants */

#define PF2 (*((volatile uint32_t *)0x40025010))

#define VP_NUM 0

/* module fields */

//uint32_t ADC_Value;
void (*ADC_Task)(uint32_t, uint32_t);
int ADC_Trigger_Change;
uint32_t data[5];
uint32_t angle[5];
uint32_t old_angle[5] = {0};

/* module internal functions */
uint32_t angle_LUT[100];

static void populate_LUT(void){
	int value = 4000;
	int step_size1 = 40;
	int step_size2 = 16;
	int i;
	for(i = 0; i < 50; i++){
		angle_LUT[i] = value;
		value -= step_size1;
	}
	for(i = 50; i < 100; i ++){
		angle_LUT[i] = value;
		value -= step_size2;
	}
}
void ADC_Read_Angle(void) {
	ADC_In5(data);
	uint32_t step = 5;	// 2
	int i;
	for(i = 0; i < 5; i++){
	    uint32_t value = data[i] / (4096 / 100);
	    value /= step;
	    angle[i] = step * value;
	}
	/*
	 * Delete after confirming above code works
	 */
	//uint32_t adc_in_val = ADC0_InSeq3();
//	uint32_t adc_in_val = data[1];
//	uint32_t value = adc_in_val / (4096 / 100);
//	value /= step;
//	return step * value;
}

void ADC_Handler(void) {
	ADC_Read_Angle();
	int i;
	for(i = 0; i < 5; i++){
	    uint32_t new_adc_value = angle[i];
	    uint32_t old_adc_value = old_angle[i];
	    uint32_t adc_val_avg = 0;
	    if(new_adc_value != old_adc_value || ADC_Trigger_Change == 0){
	        adc_val_avg = (old_adc_value + new_adc_value) / 2;
	        old_angle[i] = angle[i];
	        (*ADC_Task)(i, adc_val_avg); //index should correspond to virtual pin
	    }

	}
    /*
     * Delete after confirming above code works
     */
	/*
	uint32_t adc_val_avg = 0;
	if (new_adc_value != ADC_Value || ADC_Trigger_Change == 0) {
		ADC_Value = new_adc_value;
		adc_val_avg = (old_adc_value + new_adc_value) / 2;
		(*ADC_Task)(VP_NUM, adc_val_avg);
	}
	*/
}

/* module external methods */

void ADC_Init(uint32_t read_period, void (*task)(uint32_t, uint32_t), int trigger_on_change) {
	//ADC_Value = 0;
	ADC_Task = task;
	ADC_Trigger_Change = trigger_on_change;
	// ADC0_InitSWTriggerSeq3_Ch9();  // PE4
	//ADC0_InitSWTriggerSeq3_Ch7();  // PD0
//	populate_LUT();
	ADC_Init_5Chan();
	Timer_SetTask2(&ADC_Handler, read_period);
}


void ADC_Init_5Chan(void){
  volatile uint32_t delay;
  SYSCTL_RCGCADC_R |= 0x00000001; // 1) activate ADC0
  SYSCTL_RCGCGPIO_R |= 0x1A;      // 1) activate clock for Port B and D and E
  while((SYSCTL_PRGPIO_R&0x0A) == 0){};
  //while((SYSCTL_PRADC_R&0x0001) != 0x0001){};    // good code, but not yet implemented in simulator

  // Setting up GPIO pins for ADC input
  GPIO_PORTB_DIR_R &= ~0x30;      // 3) make PB4-5 input
  GPIO_PORTB_AFSEL_R |= 0x30;     // 4) enable alternate function on PB4-5
  GPIO_PORTB_DEN_R &= ~0x30;      // 5) disable digital I/O on PB4-5
  GPIO_PORTB_PCTL_R = GPIO_PORTB_PCTL_R&0xFF00FFFF;
  GPIO_PORTB_AMSEL_R |= 0x30;     // 6) enable analog functionality on PB4-5

  GPIO_PORTD_DIR_R &= ~0x0F;      // 3) make PD0-2 input
  GPIO_PORTD_AFSEL_R |= 0x0F;     // 4) enable alternate function on PD0-2
  GPIO_PORTD_DEN_R &= ~0x0F;      // 5) disable digital I/O on PD0-2
  GPIO_PORTD_PCTL_R = GPIO_PORTD_PCTL_R&0xFFFF0000;
  GPIO_PORTD_AMSEL_R |= 0x0F;     // 6) enable analog functionality on PD0-2

  GPIO_PORTE_DIR_R &= ~0x04;      // 3) make PD0-2 input
  GPIO_PORTE_AFSEL_R |= 0x04;     // 4) enable alternate function on PD0-2
  GPIO_PORTE_DEN_R &= ~0x04;      // 5) disable digital I/O on PD0-2
  GPIO_PORTE_PCTL_R = GPIO_PORTE_PCTL_R&0xFFFFF0FF;
  GPIO_PORTE_AMSEL_R |= 0x04;     // 6) enable analog functionality on PD0-2

  ADC0_PC_R &= ~0xF;              // 8) clear max sample rate field
  ADC0_PC_R |= 0x1;               //    configure for 125K samples/sec
  ADC0_SSPRI_R = 0x3210;          // 9) Sequencer 3 is lowest priority

  ADC0_ACTSS_R &= ~0x0001;        // 10) disable sample sequencer 0
  ADC0_EMUX_R &= ~0x000F;         // 11) seq2 is software trigger
  ADC0_SAC_R = 0x01;              // 32-point average
  ADC0_SSMUX0_R = 0x14567;         // 12) set channels for SS2
  ADC0_SSCTL0_R = 0x00600000;         // 13) no TS0 D0 IE0 END0 TS1 D1, yes IE1 END1

  ADC0_IM_R &= ~0x0001;           // 14) disable SS0 interrupts
  ADC0_ACTSS_R |= 0x0001;         // 15) enable sample sequencer 0
}

void ADC_In5(uint32_t data[6]){
  ADC0_PSSI_R = 0x0001;            // 1) initiate SS2
  while((ADC0_RIS_R&0x01)==0){};   // 2) wait for conversion done
  data[3] = ADC0_SSFIFO0_R&0xFFF;  // 3A) PE3 result
  data[2] = ADC0_SSFIFO0_R&0xFFF;  // 3A) PE2 result
  data[1] = ADC0_SSFIFO0_R&0xFFF;  // 3A) PE1 result
  data[0] = ADC0_SSFIFO0_R&0xFFF;  // 3B) PE0 result
  data[4] = ADC0_SSFIFO0_R&0xFFF;  // 3) PE4 result
  ADC0_ISC_R = 0x0001;             // 4) acknowledge completion
}
