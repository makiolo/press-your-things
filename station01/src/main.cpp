#include <Wire.h>
#include <SSD1306.h>

#define FW_NAME "station01"
#define FW_VERSION "1.0.0"
const char *__FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" FW_NAME "\x93\x44\x6b\xa7\x75";
const char *__FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" FW_VERSION "\xb0\x30\x48\xd4\x1a";

SSD1306  display(0x3c, 2, 0);

void setup()
{
	Serial.begin(115200);
	Serial.println();

	display.init();
	display.flipScreenVertically();
	display.setFont(ArialMT_Plain_10);
}

void loop()
{
	display.clear();

	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.setFont(ArialMT_Plain_16);
	display.drawString(0, 0, "linea 1");

	display.setFont(ArialMT_Plain_16);
	display.drawString(0, 16, "linea 2");

	display.setTextAlignment(TEXT_ALIGN_CENTER);
	display.setFont(ArialMT_Plain_24);
	display.drawString(128 / 2, 32, "linea 3");

	display.setTextAlignment(TEXT_ALIGN_RIGHT);
	display.drawString(10, 128, String(millis()));
	display.display();

	delay(10);
}

