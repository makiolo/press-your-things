#include <LiquidCrystal.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ACROBOTIC_SSD1306.h>
 
char* ssid = "*******";
char* password =  "************";
const char* mqttServer = "***********";
const int mqttPort = 1883;
// const char* mqttUser = "yourMQTTuser";
// const char* mqttPassword = "yourMQTTpassword";
 
WiFiClient espClient;
PubSubClient client(espClient);
unsigned char brightness = 255;
 
void setup()
{
  Wire.begin();
  oled.init();                      // Initialze SSD1306 OLED display
  oled.clearDisplay();              // Clear screen
  oled.setTextXY(0,0);              // Set cursor position, start of line 0
  oled.putString("cargando ...");
  Serial.begin(115200);
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println("Connected to the WiFi network");
 
  client.setServer(mqttServer, mqttPort);
 
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    // if (client.connect("ESP32Client", mqttUser, mqttPassword ))
    if (client.connect("ESP32Client"))
    {
      Serial.println("connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
  client.publish("esp/test", "Hello from ESP32");
  oled.clearDisplay();
}
 
void loop() {
  client.loop();
  int value = analogRead(35);
  oled.setTextXY(0,0);
  oled.putString("Alcohol:");
  oled.setTextXY(1,0);
  oled.putString("    ");
  oled.setTextXY(1,0);
  oled.putString(String(value));
  delay(700);
}
