#define PERIOD 25000
#define STARTING_DUTY 625

void PWM0A_Init(uint16_t period, uint16_t duty);

void PWM0B_Init(uint16_t period, uint16_t duty);

void PWM1A_Init(uint16_t period, uint16_t duty);

void PWM1B_Init(uint16_t period, uint16_t duty);

void PWM2A_Init(uint16_t period, uint16_t duty);

void Finger0_Duty(uint16_t angle);

void Finger1_Duty(uint16_t angle);

void Finger2_Duty(uint16_t angle);

void Finger3_Duty(uint16_t angle);

void Finger4_Duty(uint16_t angle);

void Hand_Init(void);

uint16_t toDuty (uint16_t angle);
