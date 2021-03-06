/* ADC Driver Module */

#include "ADCDriver.h"

/* constants */

#define PF2 (*((volatile uint32_t *)0x40025010))

#define VP_NUM 0

/* module fields */

uint32_t ADC_Value;
void (*ADC_Task)(uint32_t, uint32_t);
int ADC_Trigger_Change;
uint32_t data[5];

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

void ADC_In2(uint32_t data[2]){
	 ADC0_PSSI_R = 0x0008;            // 1) initiate SS3
	 while((ADC0_RIS_R&0x08)==0){};   // 2) wait for conversion done
	   // if you have an A0-A3 revision number, you need to add an 8 usec wait here
	 data[1] = ADC0_SSFIFO3_R&0xFFF;   // 3) read result
	 data[0] = ADC0_SSFIFO3_R&0xFFF;   // 3) read result
	 ADC0_ISC_R = 0x0008;             // 4) acknowledge completion
	 //return result;
}


uint32_t ADC_Read_Angle(void) {
	uint32_t step = 2;	// 5
	uint32_t adc_in_val = ADC0_InSeq3();
	uint32_t value = adc_in_val / (4096 / 100);
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
	//ADC0_InitSWTriggerSeq3_Ch7();  // PD0
	populate_LUT();
	//ADC_Init_5Chan();
	ADC0_InitSWTriggerSeq3_test(4);
	Timer_SetTask2(&ADC_Handler, read_period);
}


void ADC_Init_5Chan(void){
  volatile uint32_t delay;
  SYSCTL_RCGCADC_R |= 0x00000001; // 1) activate ADC0
  SYSCTL_RCGCGPIO_R |= 0x08;      // 1) activate clock for Port D
  while((SYSCTL_PRGPIO_R&0x10) == 0){};
  while((SYSCTL_PRADC_R&0x0001) != 0x0001){};    // good code, but not yet implemented in simulator

  // Setting up GPIO pins for ADC input
  GPIO_PORTD_DIR_R &= ~0x0F;      // 3) make PE4-0 input
  GPIO_PORTD_AFSEL_R |= 0x0F;     // 4) enable alternate function on PE4-0
  GPIO_PORTD_DEN_R &= ~0x0F;      // 5) disable digital I/O on PE4-0
  GPIO_PORTD_PCTL_R = GPIO_PORTE_PCTL_R&0xFFFF0000;
  GPIO_PORTD_AMSEL_R |= 0x0F;     // 6) enable analog functionality on PE4-0

  ADC0_PC_R &= ~0xF;              // 8) clear max sample rate field
  ADC0_PC_R |= 0x1;               //    configure for 125K samples/sec
  ADC0_SSPRI_R = 0x3210;          // 9) Sequencer 3 is lowest priority

  ADC0_ACTSS_R &= ~0x0004;        // 10) disable sample sequencer 2
  ADC0_EMUX_R &= ~0x0F00;         // 11) seq2 is software trigger
  ADC0_SAC_R = 0x06;              // 32-point average
  ADC0_SSMUX2_R = 0x0123;         // 12) set channels for SS2
  ADC0_SSCTL2_R = 0x6000;         // 13) no TS0 D0 IE0 END0 TS1 D1, yes IE1 END1
/*
  ADC0_ACTSS_R &= ~0x0008;        // 9) disable sample sequencer 3
  ADC0_EMUX_R &= ~0xF000;         // 10) seq3 is software trigger
  ADC0_SSMUX3_R &= ~0x000F;       // 11) clear SS3 field
  ADC0_SSMUX3_R += 9;             //    set channel
  ADC0_SSCTL3_R = 0x0006;         // 12) no TS0 D0, yes IE0 END0
  ADC0_IM_R &= ~0x0008;           // 13) disable SS3 interrupts
  ADC0_ACTSS_R |= 0x0008;         // 14) enable sample sequencer 3
*/
  ADC0_IM_R &= ~0x0004;           // 14) disable SS2 interrupts
  ADC0_ACTSS_R |= 0x0004;         // 15) enable sample sequencer 2
}

void ADC_In5(uint32_t data[5]){
  ADC0_PSSI_R = 0x000C;            // 1) initiate SS2
  while((ADC0_RIS_R&0x04)==0){};   // 2) wait for conversion done
  data[3] = ADC0_SSFIFO2_R&0xFFF;  // 3A) PE3 result
  data[2] = ADC0_SSFIFO2_R&0xFFF;  // 3A) PE2 result
  data[1] = ADC0_SSFIFO2_R&0xFFF;  // 3A) PE1 result
  data[0] = ADC0_SSFIFO2_R&0xFFF;  // 3B) PE0 result
  /*
  while((ADC0_RIS_R&0x08)==0){};   // 2) wait for conversion done
  data[4] = ADC0_SSFIFO3_R&0xFFF;  // 3) PE4 result
  */
  data[4] = 0;
  ADC0_ISC_R = 0x000C;             // 4) acknowledge completion
}

void ADC0_InitSWTriggerSeq3_test(unsigned char channelNum){ volatile unsigned long delay;
  switch(channelNum){             // 1) activate clock
    case 0:
    case 1:
    case 2:
    case 3:
    case 8:
    case 9:                       //    these are on GPIO_PORTE
      SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4; break;
    case 4:
    case 5:
    case 6:
    case 7:                       //    these are on GPIO_PORTD
      SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R3; break;
    case 10:
    case 11:                      //    these are on GPIO_PORTB
      SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1; break;
    default: return;              //    0 to 11 are valid channels on the LM4F120
  }
  delay = SYSCTL_RCGCGPIO_R;      // 2) allow time for clock to stabilize
  delay = SYSCTL_RCGCGPIO_R;
  delay = SYSCTL_RCGCGPIO_R;
  delay = SYSCTL_RCGCGPIO_R;
  switch(channelNum){
    case 0:                       //      Ain0 is on PE3
      GPIO_PORTE_DIR_R &= ~0x08;  // 3.0) make PE3 input
      GPIO_PORTE_AFSEL_R |= 0x08; // 4.0) enable alternate function on PE3
      GPIO_PORTE_DEN_R &= ~0x08;  // 5.0) disable digital I/O on PE3
      GPIO_PORTE_AMSEL_R |= 0x08; // 6.0) enable analog functionality on PE3
      break;
    case 1:                       //      Ain1 is on PE2
      GPIO_PORTE_DIR_R &= ~0x04;  // 3.1) make PE2 input
      GPIO_PORTE_AFSEL_R |= 0x04; // 4.1) enable alternate function on PE2
      GPIO_PORTE_DEN_R &= ~0x04;  // 5.1) disable digital I/O on PE2
      GPIO_PORTE_AMSEL_R |= 0x04; // 6.1) enable analog functionality on PE2
      break;
    case 2:                       //      Ain2 is on PE1
      GPIO_PORTE_DIR_R &= ~0x02;  // 3.2) make PE1 input
      GPIO_PORTE_AFSEL_R |= 0x02; // 4.2) enable alternate function on PE1
      GPIO_PORTE_DEN_R &= ~0x02;  // 5.2) disable digital I/O on PE1
      GPIO_PORTE_AMSEL_R |= 0x02; // 6.2) enable analog functionality on PE1
      break;
    case 3:                       //      Ain3 is on PE0
      GPIO_PORTE_DIR_R &= ~0x01;  // 3.3) make PE0 input
      GPIO_PORTE_AFSEL_R |= 0x01; // 4.3) enable alternate function on PE0
      GPIO_PORTE_DEN_R &= ~0x01;  // 5.3) disable digital I/O on PE0
      GPIO_PORTE_AMSEL_R |= 0x01; // 6.3) enable analog functionality on PE0
      break;
    case 4:                       //      Ain4 is on PD3
      GPIO_PORTD_DIR_R &= ~0x08;  // 3.4) make PD3 input
      GPIO_PORTD_AFSEL_R |= 0x08; // 4.4) enable alternate function on PD3
      GPIO_PORTD_DEN_R &= ~0x08;  // 5.4) disable digital I/O on PD3
      GPIO_PORTD_AMSEL_R |= 0x08; // 6.4) enable analog functionality on PD3
      break;
    case 5:                       //      Ain5 is on PD2
      GPIO_PORTD_DIR_R &= ~0x04;  // 3.5) make PD2 input
      GPIO_PORTD_AFSEL_R |= 0x04; // 4.5) enable alternate function on PD2
      GPIO_PORTD_DEN_R &= ~0x04;  // 5.5) disable digital I/O on PD2
      GPIO_PORTD_AMSEL_R |= 0x04; // 6.5) enable analog functionality on PD2
      break;
    case 6:                       //      Ain6 is on PD1
      GPIO_PORTD_DIR_R &= ~0x02;  // 3.6) make PD1 input
      GPIO_PORTD_AFSEL_R |= 0x02; // 4.6) enable alternate function on PD1
      GPIO_PORTD_DEN_R &= ~0x02;  // 5.6) disable digital I/O on PD1
      GPIO_PORTD_AMSEL_R |= 0x02; // 6.6) enable analog functionality on PD1
      break;
    case 7:                       //      Ain7 is on PD0
      GPIO_PORTD_DIR_R &= ~0x01;  // 3.7) make PD0 input
      GPIO_PORTD_AFSEL_R |= 0x01; // 4.7) enable alternate function on PD0
      GPIO_PORTD_DEN_R &= ~0x01;  // 5.7) disable digital I/O on PD0
      GPIO_PORTD_AMSEL_R |= 0x01; // 6.7) enable analog functionality on PD0
      break;
    case 8:                       //      Ain8 is on PE5
      GPIO_PORTE_DIR_R &= ~0x20;  // 3.8) make PE5 input
      GPIO_PORTE_AFSEL_R |= 0x20; // 4.8) enable alternate function on PE5
      GPIO_PORTE_DEN_R &= ~0x20;  // 5.8) disable digital I/O on PE5
      GPIO_PORTE_AMSEL_R |= 0x20; // 6.8) enable analog functionality on PE5
      break;
    case 9:                       //      Ain9 is on PE4
      GPIO_PORTE_DIR_R &= ~0x10;  // 3.9) make PE4 input
      GPIO_PORTE_AFSEL_R |= 0x10; // 4.9) enable alternate function on PE4
      GPIO_PORTE_DEN_R &= ~0x10;  // 5.9) disable digital I/O on PE4
      GPIO_PORTE_AMSEL_R |= 0x10; // 6.9) enable analog functionality on PE4
      break;
    case 10:                      //       Ain10 is on PB4
      GPIO_PORTB_DIR_R &= ~0x10;  // 3.10) make PB4 input
      GPIO_PORTB_AFSEL_R |= 0x10; // 4.10) enable alternate function on PB4
      GPIO_PORTB_DEN_R &= ~0x10;  // 5.10) disable digital I/O on PB4
      GPIO_PORTB_AMSEL_R |= 0x10; // 6.10) enable analog functionality on PB4
      break;
    case 11:                      //       Ain11 is on PB5
      GPIO_PORTB_DIR_R &= ~0x20;  // 3.11) make PB5 input
      GPIO_PORTB_AFSEL_R |= 0x20; // 4.11) enable alternate function on PB5
      GPIO_PORTB_DEN_R &= ~0x20;  // 5.11) disable digital I/O on PB5
      GPIO_PORTB_AMSEL_R |= 0x20; // 6.11) enable analog functionality on PB5
      break;
  }
  SYSCTL_RCGCADC_R |= 0x00000001; // 7) activate ADC0 (actually doesn't work)
 // delay = SYSCTL_RCGCADC_R;         // 8) allow time for clock to stabilize
 // delay = SYSCTL_RCGCADC_R;
 // delay = SYSCTL_RCGCADC_R;
 // delay = SYSCTL_RCGCADC_R;
  while((SYSCTL_PRADC_R&0x0001) != 0x0001){};    // good code, but not yet implemented in simulator

  ADC0_PC_R &= ~0xF;              // 9) clear max sample rate field
  ADC0_PC_R |= 0x1;               //    configure for 125K samples/sec
  ADC0_SSPRI_R = 0x3210;          // 10) Sequencer 3 is lowest priority
  ADC0_ACTSS_R &= ~0x0008;        // 11) disable sample sequencer 3
  ADC0_EMUX_R &= ~0xF000;         // 12) seq3 is software trigger
  ADC0_SSMUX3_R &= ~0x000F;       // 13) clear SS3 field
  ADC0_SSMUX3_R += channelNum;    //     set channel
  ADC0_SSCTL3_R = 0x0006;         // 14) no TS0 D0, yes IE0 END0
  ADC0_IM_R &= ~0x0008;           // 15) disable SS3 interrupts
  ADC0_ACTSS_R |= 0x0008;         // 16) enable sample sequencer 3
}



