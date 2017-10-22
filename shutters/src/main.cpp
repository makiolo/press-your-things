/*

# Homie enabled Sonoff Dual shutters

Requires the Shutters library:
https://github.com/marvinroger/arduino-shutters
and the SonoffDual library:
https://github.com/marvinroger/arduino-sonoff-dual

## Features

 * Do a short press to close shutters
 if level != 0 or open shutters if level == 0
 * Do a long press to reset

*/

#include <Homie.h>

#include <EEPROM.h>
#include <SonoffDual.h>
#include <Shutters.h>
#include <ArduinoJson.h>

#define FW_NAME "shutters"
#define FW_VERSION "1.0.0"
const char *__DEVICE_ID = "sonoff_dual";
const char *__FUNCTION_NAME = "device1";
const char *__FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" FW_NAME "\x93\x44\x6b\xa7\x75";
const char *__FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" FW_VERSION "\xb0\x30\x48\xd4\x1a";

const unsigned long COURSE_TIME = 30 * 1000;
const float CALIBRATION_RATIO = 0.1;

const bool RELAY1_MOVE = true;
const bool RELAY1_STOP = false;

const bool RELAY2_UP = true;
const bool RELAY2_DOWN = false;

const byte SHUTTERS_EEPROM_POSITION = 0;

HomieNode shuttersNode("shutters", "shutters");
StaticJsonBuffer<4000> jsonBuffer;

// Shutters

void shuttersUp() {
	SonoffDual.setRelays(RELAY1_MOVE, RELAY2_UP);
}

void shuttersDown() {
	SonoffDual.setRelays(RELAY1_MOVE, RELAY2_DOWN);
}

void shuttersHalt() {
	SonoffDual.setRelays(RELAY1_STOP, false);
}

uint8_t shuttersGetState() {
	return EEPROM.read(SHUTTERS_EEPROM_POSITION);
}

void shuttersSetState(uint8_t state) {
	EEPROM.write(SHUTTERS_EEPROM_POSITION, state);
	EEPROM.commit();
}

void onShuttersLevelReached(uint8_t level);

Shutters shutters(COURSE_TIME, shuttersUp, shuttersDown, shuttersHalt, shuttersGetState, shuttersSetState, CALIBRATION_RATIO, onShuttersLevelReached);

void onShuttersLevelReached(uint8_t level)
{
	if (shutters.isIdle()) Homie.setIdle(true); // if idle, we've reached our target
	if (Homie.isConnected()) shuttersNode.setProperty("level").send(String(level));
}

// Homie

void onHomieEvent(const HomieEvent& event)
{
	switch (event.type)
	{
		case HomieEventType::ABOUT_TO_RESET:
			shutters.reset();
			break;
	}
}

bool shuttersLevelHandler(const HomieRange& range, const String& value)
{
	for (byte i = 0; i < value.length(); i++)
	{
		if (isDigit(value.charAt(i)) == false)
			return false;
	}

	const unsigned long numericValue = value.toInt();
	if (numericValue > 100) return false;

	// wanted value is valid

	if (shutters.isIdle() && numericValue == shutters.getCurrentLevel()) return true; // nothing to do

	Homie.setIdle(false);
	shutters.setLevel(numericValue);

	return true;
}

void create_config()
{
	if (!SPIFFS.exists("/homie/config.json"))
	{
		Homie.getLogger() << "No config.json, formatting SPIFFS and creating default ..." << endl;
		SPIFFS.format();
		JsonObject& root = jsonBuffer.createObject();
		root["name"] = __FUNCTION_NAME;
		root["device_id"] = __DEVICE_ID;
		JsonObject& wifi_node = root.createNestedObject("wifi");
		wifi_node["ssid"] = "XXXXXXXXXXXXXXXXXX";
		wifi_node["password"] = "XXXXXXXXXXXXXXXXXXXXXX";
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

// Logic

void setup() {
	SonoffDual.setup();
	EEPROM.begin(4);
	shutters.begin();

	// will use SPIFFS
	SPIFFS.begin();
	create_config();

	// firmware
	Homie_setBrand(FW_NAME);
	Homie_setFirmware(FW_NAME, FW_VERSION);

	Homie.disableLogging();
	Homie.disableResetTrigger();
	Homie.setLedPin(SonoffDual.LED_PIN, SonoffDual.LED_ON);
	Homie.onEvent(onHomieEvent);

	shuttersNode.advertise("level").settable(shuttersLevelHandler);

	Homie.setup();
}

void loop() {
	shutters.loop();
	Homie.loop();
	SonoffDualButton buttonState = SonoffDual.handleButton();
	if (buttonState == SonoffDualButton::LONG) {
		Homie.reset();
	} else if (buttonState == SonoffDualButton::SHORT && shutters.isIdle()) {
		Homie.setIdle(false);

		if (shutters.getCurrentLevel() == 100) {
			shutters.setLevel(0);
		} else {
			shutters.setLevel(100);
		}
	}
}

