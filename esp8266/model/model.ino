/* GRIP SIMULATOR
 * model device mqtt client driver
  */

#include "common.h"
#include "model.h"

// variables
WiFiClient mqtt_wifi;
PubSubClient mqtt_client(mqtt_wifi);
boolean mqtt_client_online = false;
char wifi_ssid[50] = WIFI_SSID;
char mqtt_client_id[100] = MQTT_CLIENT_ID;
// leds
const int led_b_1 = LED_BUILTIN;
const int led_b_2 = 2;
// time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, TIME_SERVER);
const long timeOffset = 60 * 60 * -6;  // GMT-6=60*60*-6
int resetOffset = 10;				   // in minutes
// parsing
int mb_i = 0;
char msgbuff[MSGBUFF_MAXLEN];

// init
void setup() {
	// serial
	if (DEBUG) {
		Serial.begin(9600);
		delay(5000);
	} else
		Serial.begin(9600);
	if (DEBUG) {
		Serial.printf("\r\n");
		Serial.printf("GRIP SIMULATOR\r\n");
		Serial.printf("MODEL\r\n");
	}
	// boot
	Serial.printf("init\n");
	if (DEBUG) Serial.printf("\r\n");
	esp_boot_delay();
	// leds
	pinMode(led_b_1, OUTPUT);
	pinMode(led_b_2, OUTPUT);
	led_out_builtin(led_b_1, BRIGHTNESS_MIN);
	led_out_builtin(led_b_2, BRIGHTNESS_MIN);
	// wifi
	if (DEBUG) {
		Serial.print("[wifi] mac address: ");
		Serial.print(WiFi.macAddress().c_str());
		Serial.printf("\r\n");
		Serial.printf("[wifi] connecting to %s\r\n", &wifi_ssid);
	}
	WiFi.begin(WIFI_SSID, WIFI_PASS);
	while (WiFi.status() != WL_CONNECTED) delay(50);
	if (DEBUG) Serial.printf("[wifi] connected\r\n");
	IPAddress localIPAddress = WiFi.localIP();
	if (DEBUG) {
		Serial.printf("[wifi] ip address: ");
		Serial.print(localIPAddress);
		Serial.printf("\r\n");
	}
	// time
	if (DEBUG) Serial.printf("[time] connecting\r\n");
	timeClient.begin();
	timeClient.setTimeOffset(timeOffset);
	if (DEBUG) Serial.printf("[time] connected\r\n");
	// mqtt
	if (DEBUG) Serial.printf("[mqtt] connecting\r\n");
	sprintf(mqtt_client_id, "%s_device%d_%s", MQTT_CLIENT_ID, DEVICE_NUM, DEVICE_NAME);
	mqtt_client_online = true;
	mqtt_client.setServer(API_URL, API_PORT);
	mqtt_client.setCallback(mqtt_handler);
	// ready
	Serial.printf("ready\n");
	if (DEBUG) Serial.printf("\r\n");
}

// driver loop
void loop() {
	// onboard led output
	led_out_builtin(led_b_1, BRIGHTNESS_MAX);
	// monitor serial input
	serial_monitor();
	// reconnect/monitor mqtt client
	if (mqtt_client_online) {
		if (!mqtt_client.connected()) {
			mqtt_reconnect();
			led_out_builtin(led_b_2, BRIGHTNESS_MIN);
		} else {
			led_out_builtin(led_b_2, BRIGHTNESS_MAX);
		}
		mqtt_client.loop();
	}
	// check timer for reset
	reset_monitor();
}

// mqtt handlers
char mqtt_vpin_topic_buffer[15] = VIRTUALPIN_TOPIC;
char mqtt_vpin_value_buffer[15] = "";
void mqtt_reconnect() {
	while (!mqtt_client.connected()) {
		if (DEBUG) Serial.printf("[mqtt] attempting connection to %s:%d as \"%s\" \r\n", API_URL, API_PORT, mqtt_client_id);
		if (mqtt_client.connect(mqtt_client_id)) {
			if (DEBUG) Serial.printf("[mqtt] connected\r\n");
			for (int i = 0; i < VIRTUALPIN_COUNT; i++) {
				sprintf(mqtt_vpin_topic_buffer, "%s_%d", VIRTUALPIN_TOPIC, i);
				if (DEBUG) Serial.printf("[mqtt] subscribing to topic \"%s\"\r\n", mqtt_vpin_topic_buffer);
				mqtt_client.subscribe(mqtt_vpin_topic_buffer);
			}
		} else {
			if (DEBUG) {
				Serial.print("[mqtt] failed, rc=");
				Serial.print(mqtt_client.state());
				Serial.printf(" retrying after 5 sec\r\n");
			}
			delay(5000);
		}
	}
}
void mqtt_handler(char* topic, byte* payload, unsigned int length) {
	if (DEBUG) {
		Serial.print(F("[mqtt] message received on topic "));
		Serial.print(topic);
		Serial.print(": \"");
		for (int i = 0; i < length; i++) {
			char rC = (char)payload[i];
			Serial.print(rC);
		}
		Serial.printf("\"\r\n");
	}
	if (memcmp(topic, VIRTUALPIN_TOPIC, VIRTUALPIN_TOPIC_LEN) == 0) {
		int vp_num = atoi(topic + VIRTUALPIN_TOPIC_LEN + 1);
		int vp_val = atoi((char*)payload);
		mqtt_receive_virtualpin_update(vp_num, vp_val);
	}
}
void mqtt_update(char* topic, char* message) {
	if (mqtt_client_online && mqtt_client.connected()) {
		mqtt_client.publish(topic, message);
	}
}
void mqtt_receive_virtualpin_update(int vp_num, int vp_val) {
	if (DEBUG) Serial.printf("[mqtt] received virtual pin update: VP%d = %d\r\n", vp_num, vp_val);
	Serial.printf("%d=%d\n", vp_num, vp_val);
	if (DEBUG) Serial.printf("\r\n");
}

// serial monitor
void serial_monitor() {
	if (Serial.available()) {
		if (mb_i >= MSGBUFF_MAXLEN) {
			msgbuff[MSGBUFF_MAXLEN - 2] = '\n';
			msgbuff[MSGBUFF_MAXLEN - 1] = '\0';
			serial_monitor_input(msgbuff, mb_i);
			mb_i = 0;
		} else {
			char c = Serial.read();
			if (c != -1) {
				// Serial.print(c);
				if (c == '\n') {
					msgbuff[mb_i] = '\0';
					serial_monitor_input(msgbuff, mb_i);
					mb_i = 0;
				} else if (c != 0 && c > 32 && c < 126) {
					msgbuff[mb_i] = c;
					mb_i++;
				}
			}
		}
	}
}
void serial_monitor_input(char* msgbuff, int length) {
	if (DEBUG) Serial.println(msgbuff);
	if (memcmp(msgbuff + length - 5, "reset", 5) == 0) {
		esp_restart();
	}
}

// esp control
boolean firstTimer = 1;
unsigned long epochTime = 0;
unsigned long nextEpochTime = 0;
void reset_monitor() {
	timeClient.update();
	nextEpochTime = timeClient.getEpochTime();
	if (nextEpochTime - epochTime > resetOffset * 60) {
		epochTime = nextEpochTime;
		if (DEBUG) {
			Serial.print(F("[time] epoch time: "));
			Serial.print(epochTime);
			Serial.printf("\r\n");
		}
		if (firstTimer == 1) {
			firstTimer = 0;
			delay(500);
		} else {
			esp_restart();
		}
	}
}