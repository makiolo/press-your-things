#include <Arduino.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>

#include <Homie.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
// oled
#include <Wire.h>
#include <SSD1306.h>
// temperature
#include <Adafruit_Sensor.h>
#include <DHT.h>

#define FW_NAME "temperature"
#define FW_VERSION "2.0.1"

#ifndef DEBUG
#define DEBUG 0
#endif

const int PIN_BUTTON = D4;
const byte LED_BUILTIN1 = D4;
const char *__FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" FW_NAME "\x93\x44\x6b\xa7\x75";
const char *__FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" FW_VERSION "\xb0\x30\x48\xd4\x1a";

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

// poner en modo config (tiempo en ms manteniendo pulsado el boton)
const int reset_ticks = 10000;
const long suspend_time = 10;

unsigned long lastTemperatureSent = 0;
unsigned long lastClick = 0;

SimpleButton buttonLogic(PIN_BUTTON, true);
DHT dht(D3, DHT22);
SSD1306  display(0x3c, D2, D1);
HomieNode temperatureNode("temperature", "temperature");
HomieNode humidityNode("humidity", "humidity");
HomieNode sensationNode("sensation", "sensation");
HomieSetting<long> intervalProp("interval", "interval in measure temperatures");

float last_t = 0.0f;
float last_h = 0.0f;
float last_hic = 0.0f;

void singleclick()
{
	display.clear();

	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.setFont(ArialMT_Plain_16);
	display.drawString(0, 0, "Temp.: " + String(last_t) + "º C");

	display.setFont(ArialMT_Plain_16);
	display.drawString(0, 16, "Hume.: " + String(last_h) + "%");

	display.setTextAlignment(TEXT_ALIGN_CENTER);
	display.setFont(ArialMT_Plain_24);
	display.drawString(128 / 2, 32, String(last_hic) + "º C");

	display.display();

	lastClick = millis();
}

void loopHandler()
{
	if (millis() - lastClick >= suspend_time * 1000UL || lastClick == 0)
	{
		display.clear();
		display.display();
	}

	if (millis() - lastTemperatureSent >= intervalProp.get() * 1000UL || lastTemperatureSent == 0)
	{
		float h = dht.readHumidity();
		float t = dht.readTemperature();

		if (isnan(h) || isnan(t))
		{
			Homie.getLogger() << "Failed to read from DHT sensor!" << endl;
			return;
		}

		// Compute heat index in Celsius (isFahreheit = false)
		float hic = dht.computeHeatIndex(t, h, false);

	    Homie.getLogger() << "Temperature: " << t << " °C" << endl;
	    Homie.getLogger() << "Humidity: " << h << " °C" << endl;
	    Homie.getLogger() << "Sensation: " << hic << " °C" << endl;

		StaticJsonBuffer<200> jsonBuffer;
		JsonObject& root = jsonBuffer.createObject();
		root["temperature"] = t;
		root["Humidity"] = h;
		root["sensation"] = hic;
		String output;
		root.printTo(output);
		temperatureNode.setProperty("data").send(output);

		temperatureNode.setProperty("value").send(String(t));
		humidityNode.setProperty("value").send(String(h));
		sensationNode.setProperty("value").send(String(hic));

		last_t = t;
		last_h = h;
		last_hic = hic;

	    lastTemperatureSent = millis();
	}
	buttonLogic.tick();
	ArduinoOTA.handle();

}

void setup()
{
	Serial.begin(115200);
	Serial << endl << endl;

	// led feedback
	if(LED_BUILTIN1 > 0)
	{
		Homie.setLedPin(LED_BUILTIN1, LOW);
	}
	else
	{
		Homie.disableLedFeedback();
	}

	// reset config
	if(reset_ticks > 0)
	{
		Homie.setResetTrigger(PIN_BUTTON, LOW, reset_ticks);
	}
	else
	{
		Homie.disableResetTrigger();
	}

#if !DEBUG
	// log
	Homie.disableLogging();
#endif

	// firmware
	Homie_setBrand(FW_NAME);
	Homie_setFirmware(FW_NAME, FW_VERSION);

	// polls and callback
	Homie.setLoopFunction(loopHandler);
	temperatureNode.advertise("value");
	temperatureNode.advertise("data");
	humidityNode.advertise("value");
	sensationNode.advertise("value");

	buttonLogic.attachClick(singleclick);

	intervalProp.setDefaultValue(30).setValidator([] (long candidate) {
		return (candidate >= 0) && (candidate <= (60 * 60));
	});

	// configure homie
	Homie.setup();

	// configure OTA
	ArduinoOTA.setHostname(Homie.getConfiguration().deviceId);
	ArduinoOTA.begin();

	display.init();
	display.flipScreenVertically();
	display.setFont(ArialMT_Plain_10);
}

void loop()
{
	Homie.loop();
}

