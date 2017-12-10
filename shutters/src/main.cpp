#include <Homie.h>
#include <EEPROM.h>
#include <SonoffDual.h>
#include <Shutters.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>

#ifndef DEBUG
#define DEBUG 0
#endif

#define FW_NAME "shutters"
#define FW_VERSION "2.0.0"
const char *__FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" FW_NAME "\x93\x44\x6b\xa7\x75";
const char *__FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" FW_VERSION "\xb0\x30\x48\xd4\x1a";

const unsigned long COURSE_TIME = 22 * 1000;
const float CALIBRATION_RATIO = 0.1;

const byte SHUTTERS_EEPROM_POSITION = 0;

HomieNode shuttersNode("shutters", "shutters");

void shuttersUp() {
	SonoffDual.setRelays(false, true);
}

void shuttersDown() {
	SonoffDual.setRelays(true, false);
}

void shuttersHalt() {
	SonoffDual.setRelays(false, false);
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

// Logic
void setup() {
	SonoffDual.setup();
	EEPROM.begin(4);
	shutters.begin();

	// firmware
	Homie_setBrand(FW_NAME);
	Homie_setFirmware(FW_NAME, FW_VERSION);


#if !DEBUG
	// log
	Homie.disableLogging();
#endif
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

