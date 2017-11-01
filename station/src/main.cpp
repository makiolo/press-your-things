#include <Wire.h>
#include <SSD1306.h>
#include <DHT.h>

#define FW_NAME "station"
#define FW_VERSION "1.0.0"
const char *__FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" FW_NAME "\x93\x44\x6b\xa7\x75";
const char *__FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" FW_VERSION "\xb0\x30\x48\xd4\x1a";

SSD1306  display(0x3c, D2, D1);
DHT dht(D3, DHT22);
unsigned long boot_time = 0;
unsigned long cooldown = 30;

void setup()
{
	Serial.begin(115200);
	Serial.println();

	display.init();
	display.flipScreenVertically();
	display.setFont(ArialMT_Plain_10);

	boot_time = millis();
}

void loop()
{
	display.clear();

	float h = dht.readHumidity();
	float t = dht.readTemperature();

	if (isnan(h) || isnan(t))
	{
		Serial.println("Failed to read from DHT sensor!");
		return;
	}

	float hic = dht.computeHeatIndex(t, h, false);

	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.setFont(ArialMT_Plain_16);
	display.drawString(0, 0, "Temp.: " + String(t) + "º C");

	display.setFont(ArialMT_Plain_16);
	display.drawString(0, 16, "Hume.: " + String(h) + "%");

	display.setTextAlignment(TEXT_ALIGN_CENTER);
	display.setFont(ArialMT_Plain_24);
	display.drawString(128 / 2, 32, String(hic) + "º C");

	display.setTextAlignment(TEXT_ALIGN_RIGHT);
	display.drawString(10, 128, String(millis()));
	display.display();

	if(millis() > (boot_time + (cooldown * 1000)))
	{
		display.clear();
		display.display();
		display.end();
		// conectamos el GPIO16 (D0) puenteado con RST
		// ESP.deepSleep(20e6); // 20e6 is 20 seconds
		// conectamos al RST un boton en pull-up
		ESP.deepSleep(0);
	}

	delay(10);
}

