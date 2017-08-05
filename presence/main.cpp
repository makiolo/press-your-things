#include <Homie.h>
#include <ArduinoOTA.h>
#define FW_NAME "presence"
#define FW_VERSION "1.0.0"
const char *__FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" FW_NAME "\x93\x44\x6b\xa7\x75";
const char *__FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" FW_VERSION "\xb0\x30\x48\xd4\x1a";

HomieNode presence1("presence_1", "presence");
HomieNode presence2("presence_2", "presence");
HomieNode presence3("presence_3", "presence");
HomieNode luxNode("lux", "lux");

const int TEMPERATURE_INTERVAL = 3  * 1000UL;
const int PRESENCE_INTERVAL = 30  * 1000UL;
unsigned long lastTemperatureSent = 0;
unsigned long lastPresenceSent = 0;

void loopHandler()
{
  if (millis() - lastTemperatureSent >= TEMPERATURE_INTERVAL || lastTemperatureSent == 0)
  {
      int temperature = analogRead(0);
      Homie.getLogger() << "lux: " << temperature << endl;
      luxNode.setProperty("lux").send(String(temperature));
      lastTemperatureSent = millis();
  }

  if (millis() - lastPresenceSent >= PRESENCE_INTERVAL || lastPresenceSent == 0)
  {
      int val1 = digitalRead(D1);
      int val2 = digitalRead(D2);
      int val3 = digitalRead(D3);
    
      bool p1 = (val1 == HIGH);
      bool p2 = (val2 == HIGH);
      bool p3 = (val3 == HIGH);

      HomieInternals::Interface::get().getSendingPromise().setNode(presence1).setProperty("presence").setQos(2).setRetained(true).overwriteSetter(false).setRange({ .isRange = false, .index = 0 }).send(p1 ? "true" : "false");
      HomieInternals::Interface::get().getSendingPromise().setNode(presence2).setProperty("presence").setQos(2).setRetained(true).overwriteSetter(false).setRange({ .isRange = false, .index = 0 }).send(p2 ? "true" : "false");
      HomieInternals::Interface::get().getSendingPromise().setNode(presence3).setProperty("presence").setQos(2).setRetained(true).overwriteSetter(false).setRange({ .isRange = false, .index = 0 }).send(p3 ? "true" : "false");

      Homie.getLogger() << "<alarm> presence1: " << (p1 ? "true" : "false") << endl;
      Homie.getLogger() << "<alarm> presence2: " << (p2 ? "true" : "false") << endl;
      Homie.getLogger() << "<alarm> presence3: " << (p3 ? "true" : "false") << endl;
      
      lastPresenceSent = millis();
  }

  static bool _presence1 = false;
  static bool _presence2 = false;
  static bool _presence3 = false;

  int val1 = digitalRead(D1);
  int val2 = digitalRead(D2);
  int val3 = digitalRead(D3);

  bool p1 = (val1 == HIGH);
  bool p2 = (val2 == HIGH);
  bool p3 = (val3 == HIGH);

  if(_presence1 != p1)
  {
    HomieInternals::Interface::get().getSendingPromise().setNode(presence1).setProperty("presence").setQos(2).setRetained(true).overwriteSetter(false).setRange({ .isRange = false, .index = 0 }).send(p1 ? "true" : "false");
    _presence1 = p1;
    Homie.getLogger() << "<poll> presence1: " << (p1 ? "true" : "false") << endl;
  }

  if(_presence2 != p2)
  {
    HomieInternals::Interface::get().getSendingPromise().setNode(presence2).setProperty("presence").setQos(2).setRetained(true).overwriteSetter(false).setRange({ .isRange = false, .index = 0 }).send(p2 ? "true" : "false");
    _presence2 = p2;
    Homie.getLogger() << "<poll> presence2: " << (p2 ? "true" : "false") << endl;
  }

  if(_presence3 != p3)
  {
    HomieInternals::Interface::get().getSendingPromise().setNode(presence3).setProperty("presence").setQos(2).setRetained(true).overwriteSetter(false).setRange({ .isRange = false, .index = 0 }).send(p3 ? "true" : "false");
    _presence3 = p3;
    Homie.getLogger() << "<poll> presence3: " << (p3 ? "true" : "false") << endl;
  }

}

void setup()
{
  Serial.begin(115200);
  Serial << endl << endl;
  
  pinMode(D1, INPUT);
  pinMode(D2, INPUT);
  pinMode(D3, INPUT);

  // homie
  Homie.disableLedFeedback();
  Homie.disableLogging();
  Homie_setBrand(FW_NAME);
  Homie_setFirmware(FW_NAME, FW_VERSION);
  Homie.setLoopFunction(loopHandler);
  Homie.setup();
  ArduinoOTA.setHostname(Homie.getConfiguration().deviceId);
  ArduinoOTA.begin();
}

void loop()
{
  Homie.loop();
  ArduinoOTA.handle();
}
