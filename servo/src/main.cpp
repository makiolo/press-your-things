#include <Wire.h>
#include "SSD1306.h"
#include <Servo.h>
#include <ArduinoJson.h>
#include <Homie.h>

#define FW_NAME "servo"
#define FW_VERSION "1.0.0"
const char *__DEVICE_ID = "servo";
const char *__FUNCTION_NAME = "servo";
const char *__FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" FW_NAME "\x93\x44\x6b\xa7\x75";
const char *__FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" FW_VERSION "\xb0\x30\x48\xd4\x1a";

int PIN_SERVO = D4;
Servo servoMotor;
HomieNode lightNode("servo", "servo");
StaticJsonBuffer<4000> jsonBuffer;

bool lightOnHandler(const HomieRange& range, const String& value)
{
	if (value != "true" && value != "false") return false;

	bool on = (value == "true");
	Homie.getLogger() << "servo is " << (on ? "on" : "off") << endl;
	if(on)
	{
		// for(int i=0; i<180; i+=1)
		// {
		// 	servoMotor.write(i);
		// 	delay(15);
		// }
		// for(int i=180; i>=0; i-=1)
		// {
		// 	servoMotor.write(i);
		// 	delay(15);
		// }
	}
	Homie.getLogger() << "servo end" << endl;
	return true;
}

void create_config()
{
	SPIFFS.begin();
	if (!SPIFFS.exists("/homie/config.json"))
	{
		Homie.getLogger() << "No config.json, formatting SPIFFS and creating default ..." << endl;
		SPIFFS.format();
		JsonObject& root = jsonBuffer.createObject();
		root["name"] = __FUNCTION_NAME;
		root["device_id"] = __DEVICE_ID;
		JsonObject& wifi_node = root.createNestedObject("wifi");
		wifi_node["ssid"] = "XXXXXXXXXXXX";
		wifi_node["password"] = "XXXXXXXXXXXXXXX";
		JsonObject& mqtt_node = root.createNestedObject("mqtt");
		mqtt_node["host"] = "192.168.1.4";
		mqtt_node["port"] = 1883;
		JsonObject& ota_node = root.createNestedObject("ota");
		ota_node["enabled"] = true;

		File historyFile = SPIFFS.open("/homie/config.json", "w");
		root.printTo(historyFile);
		historyFile.close(); 
		Homie.getLogger() << "writing /homie/config.json" << endl;
		String output;
		root.printTo(output);
		Homie.getLogger() << "content config.json: " << endl;
		Homie.getLogger() << output << endl;
	}
}

void setup()
{
	Serial.begin(115200);
	Serial << endl << endl;
	create_config();

	// pinMode(PIN_SERVO, OUTPUT);
	// servoMotor.attach(PIN_SERVO);

	Homie_setBrand(FW_NAME);
	Homie_setFirmware(FW_NAME, FW_VERSION);

	lightNode.advertise("on").settable(lightOnHandler);
	Homie.setup();

	// for(int i=0; i<180; i+=1)
	// {
	// 	servoMotor.write(i);
	// 	delay(15);
	// }
	// for(int i=180; i>=0; i-=1)
	// {
	// 	servoMotor.write(i);
	// 	delay(15);
	// }
}

void loop()
{
	Homie.loop();
}

