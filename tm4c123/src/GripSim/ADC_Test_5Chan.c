#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/PLL.h"
#include "../inc/LaunchPad.h"
#include "../inc/CortexM.h"

uint32_t ADCvalue[4];

static void start_clock(void){
     volatile uint32_t delay;
     SYSCTL_RCGCADC_R |= 0x00000001; // 1) activate ADC0
     SYSCTL_RCGCGPIO_R |= 0x10;      // 1) activate clock for Port E
     while((SYSCTL_PRGPIO_R&0x10) == 0){};
     while((SYSCTL_PRADC_R&0x0001) != 0x0001){};
}

static void enable_GPIO(void){
      GPIO_PORTE_DIR_R &= ~0x0F;      // 3) make PE3-0 input
      GPIO_PORTE_AFSEL_R |= 0x0F;     // 4) enable alternate function on PE3-0
      GPIO_PORTE_DEN_R &= ~0x0F;      // 5) disable digital I/O on PE3-0
      GPIO_PORTE_PCTL_R = GPIO_PORTE_PCTL_R&0xFFFF0000;
      GPIO_PORTE_AMSEL_R |= 0x0F;     // 6) enable analog functionality on PE3-0
}

static void enable_ADC(void){
    ADC0_PC_R &= ~0xF;              // 8) clear max sample rate field
    ADC0_PC_R |= 0x1;               //125K samples/sec
    ADC0_SSPRI_R = 0x3210;          // 9) Sequencer 3 is lowest priority
    ADC0_ACTSS_R &= ~0x0004;        // 10) disable sample sequencer 2
    ADC0_EMUX_R &= ~0x0F00;         // 11) seq2 is software trigger
    ADC0_SAC_R = 0x00;              // No hardware Averaging
    ADC0_SSMUX2_R = 0x0123;         // 12) set channels for SS2
    ADC0_SSCTL2_R = 0x6000;         // 13) no TS0 D0 IE0 END0 TS1 D1, yes IE1 END1
    ADC0_IM_R &= ~0x0004;           // 14) disable SS2 interrupts
    ADC0_ACTSS_R |= 0x0004;         // 15) enable sample sequencer 2
}

void ADC_Init(void){
    start_clock();
    enable_GPIO();
    enable_ADC();
}


void ADC_In(uint32_t data[4]){
  ADC0_PSSI_R = 0x0004;            // 1) initiate SS2
  while((ADC0_RIS_R&0x04)==0){};   // 2) wait for conversion done
  data[3] = ADC0_SSFIFO2_R&0xFFF;  // 3A) PE3 result
  data[2] = ADC0_SSFIFO2_R&0xFFF;  // 3A) PE2 result
  data[1] = ADC0_SSFIFO2_R&0xFFF;  // 3A) PE1 result
  data[0] = ADC0_SSFIFO2_R&0xFFF;  // 3B) PE0 result
  ADC0_ISC_R = 0x0004;             // 4) acknowledge completion
}

int main(void){// uint32_t i;
  PLL_Init(Bus80MHz);                   // 80 MHz
  LaunchPad_Init();                     // activate port F
  ADC_Init();                         // ADC channels 8 and 9, software start
  while(1){
    PF2 = 0x04;
                          // ADCvalue[0] is ADC8 (PE5) 0 to 4095
                          // ADCvalue[1] is ADC9 (PE4) 0 to 4095
    ADC_In(ADCvalue);   // take two samples, executes in about 18.64 us
    PF2 = 0x00;
    Clock_Delay(678);     // roughly 10,000 Hz sampling, to account for 18.64 us sample time)
  }
}
