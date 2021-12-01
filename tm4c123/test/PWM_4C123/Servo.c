#include <stdint.h>
#include "Servo.h"
#include "../../inc/tm4c123gh6pm.h"

void PWM0A_Init(uint16_t period, uint16_t duty){
  SYSCTL_RCGCPWM_R |= 0x01;             // 1) activate PWM0
  SYSCTL_RCGCGPIO_R |= 0x02;            // 2) activate port B
  while((SYSCTL_PRGPIO_R&0x02) == 0){};
  GPIO_PORTB_AFSEL_R |= 0x40;           // enable alt funct on PB6
  GPIO_PORTB_PCTL_R &= ~0x0F000000;     // configure PB6 as PWM0
  GPIO_PORTB_PCTL_R |= 0x04000000;
  GPIO_PORTB_AMSEL_R &= ~0x40;          // disable analog functionality on PB6
  GPIO_PORTB_DEN_R |= 0x40;             // enable digital I/O on PB6
  SYSCTL_RCC_R = 0x00100000 |           // 3) use PWM divider
      (SYSCTL_RCC_R & (~0x00010000));   //    configure for /2 divider
  PWM0_0_CTL_R = 0;                     // 4) re-loading down-counting mode
  PWM0_0_GENA_R = 0xC8;                 // low on LOAD, high on CMPA down
  // PB6 goes low on LOAD
  // PB6 goes high on CMPA down
  PWM0_0_LOAD_R = period - 1;           // 5) cycles needed to count down to 0
  PWM0_0_CMPA_R = duty - 1;             // 6) count value when output rises
  PWM0_0_CTL_R |= 0x00000001;           // 7) start PWM0
  PWM0_ENABLE_R |= 0x00000001;          // enable PB6/M0PWM0
}

void PWM0B_Init(uint16_t period, uint16_t duty){
  volatile unsigned long delay;
  SYSCTL_RCGCPWM_R |= 0x01;             // 1) activate PWM0
  SYSCTL_RCGCGPIO_R |= 0x02;            // 2) activate port B
  delay = SYSCTL_RCGCGPIO_R;            // allow time to finish activating
  GPIO_PORTB_AFSEL_R |= 0x80;           // enable alt funct on PB7
  GPIO_PORTB_PCTL_R &= ~0xF0000000;     // configure PB7 as M0PWM1
  GPIO_PORTB_PCTL_R |= 0x40000000;
  GPIO_PORTB_AMSEL_R &= ~0x80;          // disable analog functionality on PB7
  GPIO_PORTB_DEN_R |= 0x80;             // enable digital I/O on PB7
  SYSCTL_RCC_R = 0x00100000 |           // 3) use PWM divider
      (SYSCTL_RCC_R & (~0x00010000));   //    configure for /2 divider
  PWM0_0_CTL_R = 0;                     // 4) re-loading down-counting mode
  PWM0_0_GENB_R = (PWM_0_GENB_ACTCMPBD_ONE|PWM_0_GENB_ACTLOAD_ZERO);
  // PB7 goes low on LOAD
  // PB7 goes high on CMPB down
  PWM0_0_LOAD_R = period - 1;           // 5) cycles needed to count down to 0
  PWM0_0_CMPB_R = duty - 1;             // 6) count value when output rises
  PWM0_0_CTL_R |= 0x00000001;           // 7) start PWM0
  PWM0_ENABLE_R |= 0x00000002;          // enable PB7/M0PWM1
}

void PWM1A_Init(uint16_t period, uint16_t duty){
  volatile unsigned long delay;
  SYSCTL_RCGCPWM_R |= 0x01;
  SYSCTL_RCGCGPIO_R |= 0x02;
  delay = SYSCTL_RCGCGPIO_R;
  GPIO_PORTB_AFSEL_R |= 0x10; //PB4
  GPIO_PORTB_PCTL_R &= ~0x000F0000;
  GPIO_PORTB_PCTL_R |= 0x00040000;
  GPIO_PORTB_AMSEL_R &= ~0x10;
  GPIO_PORTB_DEN_R |= 0x10;
  SYSCTL_RCC_R = 0x00100000 |
      (SYSCTL_RCC_R & (~0x00010000));
  PWM0_1_CTL_R = 0;
  PWM0_1_GENA_R = 0xC8;

  PWM0_1_LOAD_R = period - 1;
  PWM0_1_CMPA_R = duty - 1;
  PWM0_1_CTL_R |= 0x00000001;
  PWM0_ENABLE_R |= 0x00000004;
}

void PWM1B_Init(uint16_t period, uint16_t duty){
  volatile unsigned long delay;
  SYSCTL_RCGCPWM_R |= 0x01;
  SYSCTL_RCGCGPIO_R |= 0x02;
  delay = SYSCTL_RCGCGPIO_R;
  GPIO_PORTB_AFSEL_R |= 0x20; //PB5
  GPIO_PORTB_PCTL_R &= ~0x00F00000;
  GPIO_PORTB_PCTL_R |= 0x00400000;
  GPIO_PORTB_AMSEL_R &= ~0x20;
  GPIO_PORTB_DEN_R |= 0x20;
  SYSCTL_RCC_R = 0x00100000 |
      (SYSCTL_RCC_R & (~0x00010000));
  PWM0_1_CTL_R = 0;
  PWM0_1_GENB_R = 0xC08;

  PWM0_1_LOAD_R = period - 1;
  PWM0_1_CMPB_R = duty - 1;
  PWM0_1_CTL_R |= 0x00000001;
  PWM0_ENABLE_R |= 0x00000008;
}

void PWM0G_Init(uint16_t period, uint16_t duty){
  SYSCTL_RCGCPWM_R |= 0x01;             // 1) activate PWM0
  SYSCTL_RCGCGPIO_R |= 0x08;            // 2) activate port D
  while((SYSCTL_PRGPIO_R&0x08) == 0){};
  GPIO_PORTD_AFSEL_R |= 0x01;           // enable alt funct on PD0
  GPIO_PORTD_PCTL_R &= ~0x0000000F;     // configure PD0 as PWM6
  GPIO_PORTD_PCTL_R |= 0x00000004;
  GPIO_PORTD_AMSEL_R &= ~0x01;          // disable analog functionality on PD0
  GPIO_PORTD_DEN_R |= 0x01;             // enable digital I/O on PD0
  SYSCTL_RCC_R = 0x00100000 |
      (SYSCTL_RCC_R & (~0x00010000));
  PWM0_3_CTL_R = 0;                     // 4) re-loading down-counting mode
  PWM0_3_GENA_R = 0xC8;                 // low on LOAD, high on CMPA down
  // PB6 goes low on LOAD
  // PB6 goes high on CMPA down
  PWM0_3_LOAD_R = period - 1;           // 5) cycles needed to count down to 0
  PWM0_3_CMPA_R = duty - 1;             // 6) count value when output rises
  PWM0_3_CTL_R |= 0x00000001;           // 7) start PWM0
  PWM0_ENABLE_R |= 0x00000040;          // enable PD0/M0PWM6
}

void Finger0_Duty(uint16_t angle){
	uint16_t duty = toDuty(angle);
  	PWM0_0_CMPA_R = duty - 1;
}

void Finger1_Duty(uint16_t angle){
	uint16_t duty = toDuty(angle);
	PWM0_0_CMPB_R = duty - 1;
}

void Finger2_Duty(uint16_t angle){
	uint16_t duty = toDuty(angle);
	PWM0_1_CMPA_R = duty - 1;
}

void Finger3_Duty(uint16_t angle){
	uint16_t duty = toDuty(angle);
	PWM0_1_CMPB_R = duty - 1;
}

void Finger4_Duty(uint16_t angle){
	uint16_t duty = toDuty(angle);
	PWM0_3_CMPA_R = duty - 1;
}

void Hand_Init(void){
	  PWM0A_Init(PERIOD, STARTING_DUTY);
	  PWM1A_Init(PERIOD, STARTING_DUTY);
	  PWM1B_Init(PERIOD, STARTING_DUTY);
	  PWM0B_Init(PERIOD, STARTING_DUTY);
	  PWM0G_Init(PERIOD, STARTING_DUTY);
}

uint16_t toDuty (uint16_t angle){
	uint16_t duty = (11*angle) + 625;
	return duty;
}
