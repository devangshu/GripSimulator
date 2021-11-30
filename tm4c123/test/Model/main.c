#include "main.h"

int main(void)
{
	PLL_Init(Bus80MHz);
	Hand_Init();
	EnableInterrupts();

	while(1){
		// get data from ESP
		// set duty cycle of each finger
	}
}
