/* ESP8266 Driver Module */

#include "ESPDriver.h"

/* constants */

#define PF2 (*((volatile uint32_t *)0x40025010))

#define VP_COUNT 5
#define DEBUG

/* module fields */

volatile uint16_t vp0val = 0;
volatile uint16_t vp1val = 0;
volatile uint16_t vp2val = 0;
volatile uint16_t vp3val = 0;
volatile uint16_t vp4val = 0;
// These 6 variables contain the most recent ESP8266 to TM4C123 message
char serial_buf[64];
char Pin_Number[2] = "99";	// Initialize to invalid pin number
char Pin_Integer[8] = "0000";
char Pin_Float[8] = "0.0000";
uint32_t pin_num;
uint32_t pin_int;

/* module internal functions */

// send data to ESP8266 device
void TM4C_to_ESP(uint32_t pin, uint32_t value) {
	if (pin > VP_COUNT - 1) {
		return;	 // ignore illegal requests
	}
	if (pin < 10)
		ESP8266_OutChar('0');
	ESP8266_OutUDec(pin);  // Send the Virtual Pin #
	ESP8266_OutChar('=');
	ESP8266_OutUDec(value);	  // Send the current value
	ESP8266_OutString("\n");  // end message
}

// receive data from ESP8266 device
void ESP_to_TM4C_READ(void) {
#ifdef DEBUG
	int j;
	char data;
#endif

	// Check to see if a there is data in the RXD buffer
	if (ESP8266_GetMessage(serial_buf)) {  // returns false if no message
										   // Read the data from the UART5

#ifdef DEBUG
		j = 0;
		do {
			data = serial_buf[j];
			UART_OutChar(data);	 // Debug only
			j++;
		} while (data != '\n');
		UART_OutChar('\r');
#endif

		// Rip the 3 fields out of the CSV data. The sequence of data from the 8266 is:
		// Pin #, Integer Value, Float Value.
		strcpy(Pin_Number, strtok(serial_buf, "="));
		strcpy(Pin_Integer, strtok(NULL, "="));	 // Integer value that is determined by the Blynk App
		pin_num = atoi(Pin_Number);				 // Need to convert ASCII to integer
		pin_int = atoi(Pin_Integer);

		ESP_Receive_Data(pin_num, pin_int);

#ifdef DEBUG
		UART_OutString("vp");
		UART_OutString(Pin_Number);
		UART_OutString("=");
		UART_OutString(Pin_Integer);
		UART_OutString("\n\r");
#endif
	}
}

/* module external methods */

uint8_t esp_ready = 0;
void ESP_Init(uint32_t read_period, uint8_t read_enable) {
#ifdef DEBUG
	UART_Init(5);  // Enable Debug Serial Port
	UART_OutString("\n\rEE445L Lab 4D\n\r");
#endif
	ESP8266_Init();	  // Enable ESP8266 Serial Port
	ESP8266_Reset();  // Reset the WiFi module
	//ESP8266_SetupWiFi();  // Setup communications to MQTT Server

	if (read_enable) {
		// check for receive data from ESP every 10ms
		Timer_SetTask3(&ESP_to_TM4C_READ, read_period);
	}

	esp_ready = 1;
}

void ESP_Receive_Data(uint32_t pin_num, uint32_t pin_val) {
	/*
	if (pin_num == 0x01) {
		//LED = pin_int;
		//PortF_Output(LED<<2); // Blue LED

		// Parse incoming data

	} else if (pin_num == 0x02) {
		if (vp2val == 1 && pin_int == 0) {
			Switches_Handler(5);
			vp2val = 0;
		}
		if (pin_int == 1) vp2val = 1;
	} else if (pin_num == 0x03) {
		if (vp3val == 1 && pin_int == 0) {
			Switches_Handler(4);
			vp3val = 0;
		}
		if (pin_int == 1) vp3val = 1;
	} else if (pin_num == 0x04) {
		if (vp4val == 1 && pin_int == 0) {
			Switches_Handler(3);
			vp4val = 0;
		}
		if (pin_int == 1) vp4val = 1;
	} else if (pin_num == 0x05) {
		if (vp5val == 1 && pin_int == 0) {
			Switches_Handler(2);
			vp5val = 0;
		}
		if (pin_int == 1) vp5val = 1;
	} else if (pin_num == 0x06) {
		if (vp6val == 1 && pin_int == 0) {
			Switches_Handler(1);
			vp6val = 0;
		}
		if (pin_int == 1) vp6val = 1;
	} else if (pin_num == 555 || (pin_num >= 550 && pin_num <= 560)) {
		DrawHands(pin_int + 1, atoi(Pin_Float));
	}
	*/
}

void ESP_Send_Data(uint32_t pin_num, uint32_t pin_val) {
	if (pin_num == 0) {
		if (pin_val != vp0val) TM4C_to_ESP(0, pin_val);	 // VP0
		vp0val = pin_val;
	} else if (pin_num == 1) {
		if (pin_val != vp1val) TM4C_to_ESP(1, pin_val);	 // VP1
		vp1val = pin_val;
	} else if (pin_num == 2) {
		if (pin_val != vp2val) TM4C_to_ESP(2, pin_val);	 // VP2
		vp2val = pin_val;
	} else if (pin_num == 3) {
		if (pin_val != vp3val) TM4C_to_ESP(3, pin_val);	 // VP3
		vp3val = pin_val;
	} else if (pin_num == 4) {
		if (pin_val != vp4val) TM4C_to_ESP(4, pin_val);	 // VP4
		vp4val = pin_val;
	}
}
