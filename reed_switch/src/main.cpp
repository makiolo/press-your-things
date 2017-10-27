#include <Bounce2.h>
#include <Wire.h>
#include "SSD1306.h"

#define FW_NAME "reed_switch"
#define FW_VERSION "1.0.0"
const char *__FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" FW_NAME "\x93\x44\x6b\xa7\x75";
const char *__FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" FW_VERSION "\xb0\x30\x48\xd4\x1a";

SSD1306  display(0x3c, D2, D1);
Bounce debouncer = Bounce(); 
int pin = D4;

void setup()
{
	Serial.begin(115200);
	Serial.println();

	pinMode(pin, INPUT_PULLUP);
	debouncer.attach(pin);
	debouncer.interval(50);

	display.init();
	display.flipScreenVertically();
	display.setFont(ArialMT_Plain_10);

	display.clear();
	display.drawString(0, 26, "Loading ...");
	display.display();
	delay(2000);
}

// el reed switch tiene la logica invertida, eso puede dificultar el deep-sleep
void loop()
{
	debouncer.update();
	display.clear();

	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.setFont(ArialMT_Plain_24);
	if(debouncer.read() == LOW)
	{
		display.drawString(0, 26, "Iman cerca");
	}
	else
	{
		display.drawString(0, 26, "No hay iman");
	}
	display.setTextAlignment(TEXT_ALIGN_RIGHT);
	display.drawString(10, 128, String(millis()));

	display.display();
	delay(10);
}

