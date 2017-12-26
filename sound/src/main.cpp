#include <Arduino.h>
#include <Homie.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>

#define FW_NAME "sound"
#define FW_VERSION "1.0.0"

#ifndef DEBUG
#define DEBUG 0
#endif

const int PIN_ASOUND = A0;
const int PIN_DSOUND = D2;
const char *__FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" FW_NAME "\x93\x44\x6b\xa7\x75";
const char *__FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" FW_VERSION "\xb0\x30\x48\xd4\x1a";

HomieNode soundNode("sound", "sound");

int integral_sound = 0;

int digital_prev = -1;
int digital_curr = -1;

int analog_prev = -1;
int analog_curr = -1;

int counter = 0;

// const float K1 = 1.0;
// const float K2 = 0.0;

unsigned long lastSound = 0;

unsigned long prevEndSound = 0;
unsigned long lastEndSound = 0;

bool load = false;


void loopHandler()
{
	digital_prev = digital_curr;
	digital_curr = digitalRead(PIN_DSOUND);
	// HIGH --> LOW (start sound)
	// LOW ---> HIGH (end sound)
	if(digital_curr == LOW)
	{
		lastSound = millis();
		if(digital_prev == LOW)
		{
			// acumular la derivida, cuando hay previo
			analog_prev = analog_curr;
			analog_curr = analogRead(PIN_ASOUND);
			integral_sound += analog_curr; // + (K2*(analog_curr - analog_prev));
			load = true;
		}
	}
	else if(digital_curr == HIGH && ((millis() - lastSound) >= 50))
	{
		if(load)
		{
			prevEndSound = lastEndSound;
			lastEndSound = millis();
			unsigned long prev_sound = lastEndSound - prevEndSound;
			Homie.getLogger() << counter << ". Accumulated: " << String(integral_sound) << ". Prev sound: " << prev_sound << endl;

			StaticJsonBuffer<200> jsonBuffer;
			JsonObject& root = jsonBuffer.createObject();
			root["id"] = counter;
			root["accumulated"] = integral_sound;
			root["last_sound"] = prev_sound;
			String output;
			root.printTo(output);

			soundNode.setProperty("data").send( output );
			soundNode.setProperty("value").send( String(integral_sound) );
			counter += 1;
			load = false;
		}
		integral_sound = 0;
	}

	ArduinoOTA.handle();
}

void setup()
{
	Serial.begin(115200);
	Serial << endl << endl;

	pinMode (PIN_DSOUND, INPUT);
	soundNode.advertise("data");
	soundNode.advertise("value");

#if !DEBUG
	// log
	Homie.disableLogging();
#endif

	// firmware
	Homie_setBrand(FW_NAME);
	Homie_setFirmware(FW_NAME, FW_VERSION);

	// polls and callback
	Homie.setLoopFunction(loopHandler);

	// configure homie
	Homie.setup();

	// configure OTA
	ArduinoOTA.setHostname(Homie.getConfiguration().deviceId);
	ArduinoOTA.begin();
}

void loop()
{
	Homie.loop();
}

