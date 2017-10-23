#include <Arduino.h>
#include <ArduinoOTA.h>
#include <Homie.h>
#define FW_NAME "aire"
#define FW_VERSION "1.0.0"
const char *__FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" FW_NAME "\x93\x44\x6b\xa7\x75";
const char *__FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" FW_VERSION "\xb0\x30\x48\xd4\x1a";

#include <Servo.h>
#include <DHT.h>
#define DHTPIN D4     // what digital pin the DHT22 is conected to
#define DHTTYPE DHT22   // there are multiple kinds of DHT sensors

const int TEMPERATURE_INTERVAL = 10 * 1000UL;
unsigned long lastTemperatureSent = 0;
const int PIN_SERVO = D1;
const int PIN_RELAY = D0;
const int PIN_DUST = D6;

DHT dht(DHTPIN, DHTTYPE);
Servo servoMotor;
int request_button_state = 0;
int angle_min = 45;
int angle_max = 180 - angle_min;

HomieNode lightNode("button", "button");
HomieNode temperatureNode("temperature", "temperature");
HomieNode humidityNode("humidity", "humidity");
HomieNode heatIndexNode("heat_index", "heat_index");
HomieNode dustNode("dust", "dust");

void push_button()
{
  if(request_button_state != 0)
  {
    Homie.getLogger() << "state: " << request_button_state << endl;
  }
  switch(request_button_state)
  {
    case 0:
    {
      return;
    }
    case 1:
    {
        Homie.getLogger() << "begin push button" << endl;
        HomieInternals::Interface::get().getSendingPromise().setNode(lightNode).setProperty("on").setQos(2).setRetained(true).overwriteSetter(true).setRange({ .isRange = false, .index = 0 }).send("true");
        digitalWrite(PIN_RELAY, LOW);
        request_button_state++;
        return;
     }
    case 2:
    {
        servoMotor.write(angle_min);
        request_button_state++;
        return;
     }
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    {
        delay(100);
        request_button_state++;
        return;
     }
    case 9:
    {
        servoMotor.write(angle_max);
        request_button_state++;
        return;
     }
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    {
        delay(100);
        request_button_state++;
        return;
     }
    case 16:
    {
        servoMotor.write(angle_min);
        request_button_state++;
        return;
     }
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    {
        delay(100);
        request_button_state++;
        return;
     }
    case 23:
    {
        Homie.getLogger() << "end push button" << endl;
        //
        request_button_state = 0;
        HomieInternals::Interface::get().getSendingPromise().setNode(lightNode).setProperty("on").setQos(2).setRetained(true).overwriteSetter(true).setRange({ .isRange = false, .index = 0 }).send("false");
        digitalWrite(PIN_RELAY, HIGH);
        return;
     }
  }
}

const int total_buffer = 6;
float data[total_buffer];
int data_index = 0;

void loopHandler() {
  if (millis() - lastTemperatureSent >= TEMPERATURE_INTERVAL || lastTemperatureSent == 0)
  {
    digitalWrite(PIN_DUST, LOW); // power on the LED
    delayMicroseconds(280);
    data[data_index] = analogRead(0);
    delayMicroseconds(40);
    digitalWrite(PIN_DUST, HIGH); // turn the LED off

    float average = 0;
    for(int i = 0; i < total_buffer; ++i)
    {
        average += data[i];
    }
    average /= total_buffer;
   
    dustNode.setProperty("density").send(String(average));
    data_index++;
    if(data_index >= total_buffer)
    {
      data_index = 0;
    }

    //////////////////////////////////
    
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      lastTemperatureSent = 0;
      return;
    }

    // Compute heat index in Celsius (isFahreheit = false)
    float hic = dht.computeHeatIndex(t, h, false);

    Homie.getLogger() << "temperature: " << t << endl;
    Homie.getLogger() << "humidity: " << h << endl;
    Homie.getLogger() << "sensation: " << hic << endl;

    temperatureNode.setProperty("temperature").send(String(t));
    humidityNode.setProperty("humidity").send(String(h));
    heatIndexNode.setProperty("heat_index").send(String(hic));

    lastTemperatureSent = millis();
  }
}

bool nodeHandler(const HomieRange& range, const String& value)
{
  if((request_button_state == 0) && (value == "true"))
  {
    request_button_state = 1;
  }
  return true;
}

void setup()
{
  Serial.begin(115200);
  Serial << endl << endl;
  pinMode(PIN_DUST, OUTPUT);
  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, HIGH);

  // servo
  pinMode(PIN_SERVO, OUTPUT);
  servoMotor.attach(PIN_SERVO);
  servoMotor.write(angle_min);

  // homie
  Homie.disableLogging();
  Homie_setBrand(FW_NAME);
  Homie_setFirmware(FW_NAME, FW_VERSION);
  Homie.setLoopFunction(loopHandler);
  lightNode.advertise("on").settable(nodeHandler);
  Homie.setup();
  ArduinoOTA.setHostname(Homie.getConfiguration().deviceId);
  ArduinoOTA.begin();
}

void loop()
{
  Homie.loop();
  ArduinoOTA.handle();
  push_button();
}
