#include <Arduino.h>
#include <Homie.h>
#include <ArduinoOTA.h>

#ifndef DEBUG
#define DEBUG 0
#endif

#define FW_NAME "aire"
#define FW_VERSION "2.0.0"
const char *__FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" FW_NAME "\x93\x44\x6b\xa7\x75";
const char *__FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" FW_VERSION "\xb0\x30\x48\xd4\x1a";

HomieNode buttonNode("button", "button");

class pusher
{
public:
	explicit pusher()
		: _state(7)
	{
		;
	}

	void begin()
	{
		pinMode (D3, OUTPUT);
		pinMode (D4, OUTPUT);
		stop();
	}

	void turn_left()
	{
		digitalWrite(D3, HIGH);
		digitalWrite(D4, LOW);
	}

	void turn_right()
	{
		digitalWrite(D3, LOW);
		digitalWrite(D4, HIGH);
	}

	void stop()
	{
		digitalWrite(D3, LOW);
		digitalWrite(D4, LOW);
	}

	void push_button()
	{
		if(_state == 7)
		{
			Homie.getLogger() << "push button!" << endl;
			_state = 0;
		}
	}

	void cancel()
	{
		_state = 7;
	}

	void update()
	{
		if(_state == 0)
		{
			Homie.getLogger() << "state 0 !" << endl;
			buttonNode.setProperty("doing").send("true");
			turn_left();
			_stamp = millis();
			_state = 1;
		}
		else if(_state == 1)
		{
			Homie.getLogger() << "state 1 !" << endl;
			delay(300);
			_stamp = millis();
			_state = 2;
		}
		else if(_state == 2)
		{
			Homie.getLogger() << "state 2 !" << endl;
			turn_right();
			_stamp = millis();
			_state = 3;
		}
		else if(_state == 3)
		{
			Homie.getLogger() << "state 3 !" << endl;
			delay(800);
			_stamp = millis();
			_state = 4;
		}
		else if(_state == 4)
		{
			Homie.getLogger() << "state 4 !" << endl;
			turn_left();
			_stamp = millis();
			_state = 5;
		}
		else if(_state == 5)
		{
			Homie.getLogger() << "state 5 !" << endl;
			delay(50);
			_stamp = millis();
			_state = 6;
		}
		else if(_state == 6)
		{
			Homie.getLogger() << "state 6 !" << endl;
			stop();
			buttonNode.setProperty("doing").send("false");
			_stamp = millis();
			_state = 7;
		}
	}

	int _state;
	// tiempo desde el estado anterior
	unsigned long _stamp;
};

pusher p;

void loopHandler()
{
	p.update();
	ArduinoOTA.handle();
}

bool nodeHandler(const HomieRange& range, const String& value)
{
	if(value == "true")
	{
		p.push_button();
	}
	else if(value == "false")
	{
		p.cancel();
	}

	return true;
}

void setup()
{
	Serial.begin(115200);
	Serial << endl << endl;

	pinMode (D3, OUTPUT);
	pinMode (D4, OUTPUT);
	digitalWrite(D3, LOW);
	digitalWrite(D4, LOW);

	int pinCLK = D7;
	int pinDT = D6;
	int pinBUTTON = D5;
	pinMode (pinCLK, INPUT);
	pinMode (pinDT, INPUT);
	pinMode (pinBUTTON, INPUT_PULLUP);

	// reset config
	Homie.disableResetTrigger();

#if !DEBUG
	// log
	Homie.disableLogging();
#endif

	// firmware
	Homie_setBrand(FW_NAME);
	Homie_setFirmware(FW_NAME, FW_VERSION);

	// polls and callback
	Homie.setLoopFunction(loopHandler);
	buttonNode.advertise("request").settable(nodeHandler);
	buttonNode.advertise("doing");

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

