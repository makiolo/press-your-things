#include <Arduino.h>
#include <Homie.h>
#include <ArduinoOTA.h>
#include <OneButton.h>
#include <Bounce2.h>

#define FW_NAME "button"
#define FW_VERSION "1.5.0"
const char *__FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" FW_NAME "\x93\x44\x6b\xa7\x75";
const char *__FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" FW_VERSION "\xb0\x30\x48\xd4\x1a";

#ifndef SONOFF
#define SONOFF 0
#endif

#ifndef DEBUG
#define DEBUG 0
#endif

const int PIN_BUTTON = 0;
#if SONOFF
const int PIN_RELAY = 12;
const bool inverse_logic_relay = false;
const byte LED_BUILTIN1 = 13;
#else
const int PIN_RELAY = 2;
const bool inverse_logic_relay = false;
const byte LED_BUILTIN1 = 2;
#endif

// 50 ms in bound
const int bounce_ticks = 50;
// poner en modo config (tiempo en ms manteniendo pulsado el boton)
const int reset_ticks = 10000;
// press_ticks: tiempo para ser considerada larga pulsacion
const int press_ticks = 1000;
// click_ticks: tiempo que pasa antes de que un click es detectado
const int click_ticks = 600;

OneButton button(PIN_BUTTON, false);
HomieNode buttonNode("button", "button0");
Bounce debouncer = Bounce(); 

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
	int value = debouncer.read();
	_press(inverse_logic_relay ? value == HIGH : value == LOW);
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
	press_on();
}

// manual doble click
void doubleclick()
{
	toggle();
}

// manual long click
void longclick()
{
	press_off();
}

void setup()
{
	Serial.begin(115200);
	Serial << endl << endl;

	// rele
	pinMode(PIN_RELAY, OUTPUT);

	debouncer.attach(PIN_RELAY);
	debouncer.interval(bounce_ticks);
	button.setDebounceTicks(bounce_ticks);
	button.setClickTicks(click_ticks);
	button.setPressTicks(press_ticks);

	// led feedback
	Homie.setLedPin(LED_BUILTIN1, LOW);
	// Homie.disableLedFeedback();
	
	// reset config
	Homie.setResetTrigger(PIN_BUTTON, HIGH, reset_ticks);
	// Homie.disableResetTrigger();
	
	// log
	Homie.disableLogging();

	Homie_setBrand(FW_NAME);
	Homie_setFirmware(FW_NAME, FW_VERSION);
	buttonNode.advertise("on").settable(switchOnHandler);
	Homie.setup();
	ArduinoOTA.setHostname(Homie.getConfiguration().deviceId);
	ArduinoOTA.begin();

	button.attachClick(singleclick);
	button.attachDoubleClick(doubleclick);
	button.attachLongPressStop(longclick);
}


void led_on()
{
	digitalWrite(LED_BUILTIN1, LOW);
}

void led_off()
{
	digitalWrite(LED_BUILTIN1, HIGH);
}

void loop()
{
	Homie.loop();
	if (Homie.isConfigured())
	{
		// The device is configured, in normal mode
		if (Homie.isConnected())
		{
			// The device is connected
			button.tick();
			debouncer.update();
			ArduinoOTA.handle();
		}
		else
		{
			// The device is not connected
			led_on();
			delay(1000);
			led_off();
			delay(2000);
		}
	}
	else
	{
		// The device is not configured, in either configuration or standalone mode
		led_on();
		delay(100);
		led_off();
		delay(200);
	}
}

