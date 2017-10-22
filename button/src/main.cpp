#include <Arduino.h>
#include <Homie.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>

#define FW_NAME "button"
#define FW_VERSION "2.0.0"

#ifndef SONOFF
#define SONOFF 0
#endif

#if SONOFF
const int PIN_BUTTON = 0;
const int PIN_RELAY = 12;
const bool inverse_logic_relay = false;
const byte LED_BUILTIN1 = 13;
const char *__DEVICE_ID = "sonoff";
#else
const int PIN_BUTTON = 0;
const int PIN_RELAY = 2;
const bool inverse_logic_relay = false;
const byte LED_BUILTIN1 = 1;
const char *__DEVICE_ID = "custom";
#endif
const char *__FUNCTION_NAME = "device1";
const char *__FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" FW_NAME "\x93\x44\x6b\xa7\x75";
const char *__FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" FW_VERSION "\xb0\x30\x48\xd4\x1a";

// 50 ms in bound
const int bounce_ticks = 20;
// poner en modo config (tiempo en ms manteniendo pulsado el boton)
const int reset_ticks = 10000;
// press_ticks: tiempo para ser considerada larga pulsacion
const int press_ticks = 1500;
// click_ticks: tiempo que pasa antes de que un click es detectado
const int click_ticks = 100;

extern "C" {
	typedef void (*callbackFunction)(void);
}

class SimpleButton
{
public:
	explicit SimpleButton(int pin, bool momentary)
		: _pin(pin)
		, _button_down_time(0)
		, _last_button_state(1)
		, _button_press_handled(0)
		, _momentary(momentary)
	{
		if(momentary)
			pinMode(_pin, INPUT);
		else
			pinMode(_pin, INPUT_PULLUP);
	}

	void attachClick(callbackFunction newFunction)
	{
		_clickFunc = newFunction;
	}

	void attachDoubleClick(callbackFunction newFunction)
	{
		// TODO:
	}

	void attachLongPressStop(callbackFunction newFunction)
	{
		// TODO:
	}

	void tick()
	{
		byte buttonState = digitalRead(_pin);
		// momentaneo
		// hace trigger cuando pasa de LOW a HIGH dentro de unas restricciones de tiempo
		// en no momentaneo hace trigger en cada cambio de estado.
		if ( buttonState != _last_button_state )
		{
			if(_momentary)
			{
				if (buttonState == LOW)
				{
					_button_down_time     = millis();
					_button_press_handled = 0;
				}
				else
				{
					unsigned long dt = millis() - _button_down_time;
					if ( dt >= 90 && dt <= 950 && _button_press_handled == 0 )
					{
						if (_clickFunc)
						{
							_clickFunc();
						}
						_button_press_handled = 1;
					}
				}
			}
			else
			{
				if (_clickFunc)
				{
					_clickFunc();
				}
			}
			_last_button_state = buttonState;
		}
	}
protected:
	int _pin;
	unsigned long _button_down_time;
	byte _last_button_state;
	byte _button_press_handled;
	callbackFunction _clickFunc;
	bool _momentary;
};

SimpleButton buttonLogic(PIN_BUTTON, true);
SimpleButton buttonLogic2(14, false);
HomieNode buttonNode(__FUNCTION_NAME, __DEVICE_ID);
StaticJsonBuffer<4000> jsonBuffer;

void led_on()
{
	// Homie.getLogger() << "led on." << endl;
	digitalWrite(LED_BUILTIN1, LOW);
}

void led_off()
{
	// Homie.getLogger() << "led off." << endl;
	digitalWrite(LED_BUILTIN1, HIGH);
}

void blink(int time_on, int time_off)
{
	led_on();
	delay(time_on);
	led_off();
	delay(time_off);
}

void _press(bool on)
{
	if(inverse_logic_relay)
	{
		digitalWrite(PIN_RELAY, on ? LOW : HIGH);
	}
	else
	{
		digitalWrite(PIN_RELAY, on ? HIGH : LOW);
	}
	buttonNode.setProperty("on").send(on ? "true" : "false");
	Homie.getLogger() << "Switch is " << (on ? "on" : "off") << endl;
}

void toggle()
{
	if(inverse_logic_relay)
	{
		_press(digitalRead(PIN_RELAY) == HIGH);
	}
	else
	{
		_press(digitalRead(PIN_RELAY) == LOW);
	}
}

void press_on()
{
	_press(true);
}

void press_off()
{
	_press(false);
}

// remote control
bool switchOnHandler(HomieRange range, String value)
{
	if (value == "toggle")
	{
		toggle();
		return true;
	}
	else if (value == "true")
	{
		press_on();
		return true;
	}
	else if (value == "false")
	{
		press_off();
		return true;
	}
	return false;
}

// manual click
void singleclick()
{
	Homie.getLogger() << "triggered single click." << endl;
	toggle();
}

// manual doble click
void doubleclick()
{
	Homie.getLogger() << "triggered double click." << endl;
	press_on();
}

// manual long click
void longclick()
{
	Homie.getLogger() << "triggered long click." << endl;
	press_off();
}

void loopHandler()
{
	buttonLogic.tick();
	buttonLogic2.tick();
	ArduinoOTA.handle();
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
		wifi_node["ssid"] = "MOVISTAR_92B3";
		wifi_node["password"] = "5FAD3C90675A7365A3A9";
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

	// will use SPIFFS
	SPIFFS.begin();
	create_config();

	// config rele
	pinMode(PIN_RELAY, OUTPUT);
	digitalWrite(PIN_RELAY, LOW);

	// led feedback
	Homie.setLedPin(LED_BUILTIN1, LOW);
	// Homie.disableLedFeedback();

	// reset config
	Homie.setResetTrigger(PIN_BUTTON, LOW, reset_ticks);
	// Homie.disableResetTrigger();

	// log
	Homie.disableLogging();

	// firmware
	Homie_setBrand(FW_NAME);
	Homie_setFirmware(FW_NAME, FW_VERSION);

	// polls and callback
	Homie.setLoopFunction(loopHandler);
	buttonNode.advertise("on").settable(switchOnHandler);

	// configurar boton
	buttonLogic.attachClick(singleclick);
	buttonLogic2.attachClick(singleclick);
	// buttonLogic.attachDoubleClick(doubleclick);
	// buttonLogic.attachLongPressStop(longclick);
	// buttonLogic.setDebounceTicks(bounce_ticks);
	// buttonLogic.setClickTicks(click_ticks);
	// buttonLogic.setPressTicks(press_ticks);

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

