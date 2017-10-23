#include <ESP8266WiFi.h> 
#include "./functions.h" 
#define disable 0 
#define enable  1 

#define FW_NAME "sniffer"
#define FW_VERSION "1.0.0"
const char *__FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" FW_NAME "\x93\x44\x6b\xa7\x75";
const char *__FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" FW_VERSION "\xb0\x30\x48\xd4\x1a";

// uint8_t channel = 1; 
unsigned int channel = 1; 

void setup() { 
	Serial.begin(57600); 
	Serial.printf("\n\nSDK version:%s\n\r", system_get_sdk_version()); 
	Serial.println(F("ESP8266 mini-sniff by Ray Burnette http://www.hackster.io/rayburne/projects")); 
	Serial.println(F("Type:   /-------MAC------/-----WiFi Access Point SSID-----/  /----MAC---/  Chnl  RSSI")); 
	wifi_set_opmode(STATION_MODE);            // Promiscuous station mode 
	wifi_set_channel(channel); 
	wifi_promiscuous_enable(disable); 
	wifi_set_promiscuous_rx_cb(promisc_cb);   // Set up promiscuous callback 
	wifi_promiscuous_enable(enable); 
} 

void loop() { 
	channel = 1; 
	wifi_set_channel(channel); 
	while (true) { 
		nothing_new++;                          // Check bounds 
		if (nothing_new > 200) { 
			nothing_new = 0; 
			channel++; 
			if (channel == 15) break;             // Only scan channels 1 to 14 
			wifi_set_channel(channel); 
		} 
		delay(1);                              // critical processing timeslice 
	} 
} 

