#include <Homie.h>
#include <Servo.h>

/*
weird pin in NodeMCU
static const uint8_t D0   = 16;
static const uint8_t D1   = 5;
static const uint8_t D2   = 4;
static const uint8_t D3   = 0;
static const uint8_t D4   = 2;
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 15;
static const uint8_t D9   = 3;
static const uint8_t D10  = 1;
 */
const int PIN_TRANSISTOR = 0;
const int PIN_SERVO = 2;

Servo servoMotor;
int request_button_state = 0;

HomieNode lightNode("light", "switch");
HomieNode luxNode("lux", "lux");

void onHomieEvent(const HomieEvent& event) {
  switch (event.type) {
    case HomieEventType::STANDALONE_MODE:
      Serial << "Standalone mode started" << endl;
      break;
    case HomieEventType::CONFIGURATION_MODE:
      Serial << "Configuration mode started" << endl;
      break;
    case HomieEventType::NORMAL_MODE:
      Serial << "Normal mode started" << endl;
      break;
    case HomieEventType::OTA_STARTED:
      Serial << "OTA started" << endl;
      break;
    case HomieEventType::OTA_FAILED:
      Serial << "OTA failed" << endl;
      break;
    case HomieEventType::OTA_SUCCESSFUL:
      Serial << "OTA successful" << endl;
      break;
    case HomieEventType::ABOUT_TO_RESET:
      Serial << "About to reset" << endl;
      break;
    case HomieEventType::WIFI_CONNECTED:
      Serial << "Wi-Fi connected, IP: " << event.ip << ", gateway: " << event.gateway << ", mask: " << event.mask << endl;
      break;
    case HomieEventType::WIFI_DISCONNECTED:
      Serial << "Wi-Fi disconnected, reason: " << (int8_t)event.wifiReason << endl;
      break;
    case HomieEventType::MQTT_CONNECTED:
      Serial << "MQTT connected" << endl;
      break;
    case HomieEventType::MQTT_DISCONNECTED:
      Serial << "MQTT disconnected, reason: " << (int8_t)event.mqttReason << endl;
      break;
    case HomieEventType::MQTT_PACKET_ACKNOWLEDGED:
      Serial << "MQTT packet acknowledged, packetId: " << event.packetId << endl;
      break;
    case HomieEventType::READY_TO_SLEEP:
      Serial << "Ready to sleep" << endl;
      break;
  }
}

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
        request_button_state++;
        return;
     }
    case 2:
    {
        servoMotor.write(0);
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
        servoMotor.write(180);
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
        servoMotor.write(0);
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
        delay(400);
        request_button_state++;
        return;
     }
    case 23:
    {
        Homie.getLogger() << "end push button" << endl;
        //
        request_button_state = 0;
        HomieInternals::Interface::get().getSendingPromise().setNode(lightNode).setProperty("on").setQos(2).setRetained(true).overwriteSetter(true).setRange({ .isRange = false, .index = 0 }).send("false");
        return;
     }
  }
}

const int TEMPERATURE_INTERVAL = 3;
unsigned long lastTemperatureSent = 0;

void loopHandler() {
  if (millis() - lastTemperatureSent >= TEMPERATURE_INTERVAL * 1000UL || lastTemperatureSent == 0) {
    int temperature = analogRead(0);
    Homie.getLogger() << "lux: " << temperature << " °C" << endl;
    luxNode.setProperty("lux").send(String(temperature));
    lastTemperatureSent = millis();
  }
}

// listen topic: homie/5ccf7f3d53a6/light/on/set
bool nodeHandler(const HomieRange& range, const String& value)
{
  // Homie.getLogger() << "Received on node " << node.getId() << ": " << property << " = " << value << endl;
  if((request_button_state == 0) && (value == "true"))
  {
    request_button_state = 1;
  }
  return true;
}

bool broadcastHandler(const String& level, const String& value) {
  Homie.getLogger() << "Received broadcast level " << level << ": " << value << endl;
  return true;
}

void setup()
{
  Serial.begin(115200);
  Serial << endl << endl;
  
  // servo
  pinMode(PIN_SERVO, OUTPUT);
  servoMotor.attach(PIN_SERVO);
  servoMotor.write(0);

  // homie
  //Homie.disableLogging();
  Homie_setBrand("AirAcondWifiDev");
  Homie_setFirmware("aire-acond-dev", "0.0.4");
  Homie.setLoopFunction(loopHandler);
  lightNode.advertise("on").settable(nodeHandler);
  Homie.onEvent(onHomieEvent);
  Homie.setup();
}

void loop()
{
  Homie.loop();
  push_button();
}
