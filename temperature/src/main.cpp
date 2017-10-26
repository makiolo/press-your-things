#include <DHT.h>

#define FW_NAME "temperature"
#define FW_VERSION "1.0.0"
const char *__FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" FW_NAME "\x93\x44\x6b\xa7\x75";
const char *__FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" FW_VERSION "\xb0\x30\x48\xd4\x1a";

DHT dht(D3, DHT22);

void setup(void)
{
	Serial.begin(9600);
}

void loop(void)
{
	float h = dht.readHumidity();
	float t = dht.readTemperature();

	if (isnan(h) || isnan(t))
	{
		Serial.println("Failed to read from DHT sensor!");
		return;
	}

	// Compute heat index in Celsius (isFahreheit = false)
	float hic = dht.computeHeatIndex(t, h, false);

	Serial.println("temperature: ");
	Serial.println(t);
	Serial.println("humidity: ");
	Serial.println(h);
	Serial.println("sensation: ");
	Serial.println(hic);
	delay(100); 
}
