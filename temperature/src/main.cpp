#include <DHT.h>
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

