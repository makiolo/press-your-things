#include <Arduino.h>
#include <Homie.h>
#include <ArduinoOTA.h>
#include <Bounce2.h>

#define FW_NAME "presence"
#define FW_VERSION "2.0.1"

const char *__FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" FW_NAME "\x93\x44\x6b\xa7\x75";
const char *__FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" FW_VERSION "\xb0\x30\x48\xd4\x1a";

HomieNode presence1("presence1", "presence");
HomieNode presence2("presence2", "presence");
HomieNode presence3("presence3", "presence");

const int PRESENCE_INTERVAL = 30  * 1000UL;
unsigned long lastPresenceSent = 0;
Bounce deb1;
Bounce deb2;
Bounce deb3;

void loopHandler()
{
	bool changed1 = deb1.update();
	bool changed2 = deb2.update();
	bool changed3 = deb3.update();
	bool forced = false;

	if (millis() - lastPresenceSent >= PRESENCE_INTERVAL || lastPresenceSent == 0)
	{
		forced = true;
		lastPresenceSent = millis();
	}

	if ( changed1 || forced )
	{
		bool p1 = (deb1.read() == HIGH);
		presence1.setProperty("value").send(p1 ? "1.0" : "0.0");
		Homie.getLogger() << "presence1: " << (p1 ? "1.0" : "0.0") << endl;
	}

	if ( changed2 || forced )
	{
		bool p2 = (deb2.read() == HIGH);
		presence2.setProperty("value").send(p2 ? "1.0" : "0.0");
		Homie.getLogger() << "presence2: " << (p2 ? "1.0" : "0.0") << endl;
	}

	if ( changed3 || forced )
	{
		bool p3 = (deb3.read() == HIGH);
		presence3.setProperty("value").send(p3 ? "1.0" : "0.0");
		Homie.getLogger() << "presence3: " << (p3 ? "1.0" : "0.0") << endl;
	}

	ArduinoOTA.handle();
}

void setup()
{
	Serial.begin(115200);
	Serial << endl << endl;

	deb1.attach(D5, INPUT);
	deb2.attach(D6, INPUT);
	deb3.attach(D7, INPUT);
	deb1.interval(50);
	deb2.interval(50);
	deb3.interval(50);

	presence1.advertise("value");
	presence2.advertise("value");
	presence3.advertise("value");

	// Homie.disableLedFeedback();
	// Homie.disableLogging();

	Homie_setBrand(FW_NAME);
	Homie_setFirmware(FW_NAME, FW_VERSION);
	Homie.setLoopFunction(loopHandler);
	Homie.setup();

	ArduinoOTA.setHostname(Homie.getConfiguration().deviceId);
	ArduinoOTA.begin();
}

void loop()
{
	Homie.loop();
}

