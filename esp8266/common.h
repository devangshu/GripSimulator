/* grip simulator common device client config */

#ifndef __GS_COMMON_H__
#define __GS_COMMON_H__

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>

#define DEBUG false
#define DEBUG_VERBOSE false

//#define WIFI_SSID "21 Resident"
//#define WIFI_PASS "Hookem*2101"
#define WIFI_SSID "utexas-iot"
#define TIME_SERVER "pool.ntp.org"

#define MQTT_CLIENT_ID "gripsim_445l_client"

#define VIRTUALPIN_COUNT 5
#define VIRTUALPIN_TOPIC "virtualpin"
#define VIRTUALPIN_TOPIC_LEN 10

#define MSGBUFF_MAXLEN 500

#define BRIGHTNESS_MIN 0
#define BRIGHTNESS_MAX PWMRANGE	 // 1023 or 255

/* utilities */
// generic
int bound_integer(int value, int minimum, int maximum) {
	if (value < minimum) value = minimum;
	if (value > maximum) value = maximum;
	return value;
}
// esp control
void esp_boot_delay() {
	for (uint8_t t = 2; t > 0; t--) {
		if (DEBUG) {
			Serial.printf("[boot] wait %d\r\n", t);
			Serial.flush();
		}
		delay(1000);
	}
}
void esp_restart() {
	if (DEBUG) {
		Serial.print(F("[boot] resetting..."));
		Serial.printf("\r\n");
	}
	ESP.restart();
}
// led control
int correct_brightness(int brightness) {
	return bound_integer(brightness, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
}
void led_out(int led_pin, int brightness) {
	brightness = correct_brightness(brightness);
	analogWrite(led_pin, brightness);
}
void led_out_builtin(int led_b_pin, int brightness) {
	brightness = correct_brightness(brightness);
	brightness = BRIGHTNESS_MAX - (brightness - BRIGHTNESS_MIN);
	analogWrite(led_b_pin, brightness);
}

#endif
