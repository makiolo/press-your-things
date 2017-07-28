#include <Arduino.h>
#include <Homie.h>

#define FW_NAME "presence"
#define FW_VERSION "1.0.1"
const char *__FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" FW_NAME "\x93\x44\x6b\xa7\x75";
const char *__FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" FW_VERSION "\xb0\x30\x48\xd4\x1a";

HomieNode presence1("presence_1", "presence");
HomieNode presence2("presence_2", "presence");
HomieNode presence3("presence_3", "presence");
HomieNode luxNode("lux_1", "lux");

const int LUX_INTERVAL = 3  * 1000UL;
const int PRESENCE_INTERVAL = 60 * 1000UL;
unsigned long lux_mark = 0;
unsigned long presence_mark = 0;

void send_presence(bool force = false)
{
  static bool _presence1 = false;
  static bool _presence2 = false;
  static bool _presence3 = false;

  int val1 = digitalRead(D1);
  int val2 = digitalRead(D2);
  int val3 = digitalRead(D4);

  bool p1 = (val1 == HIGH);
  bool p2 = (val2 == HIGH);
  bool p3 = (val3 == HIGH);

  if(force || (_presence1 != p1))
  {
    presence1.setProperty("presence").send(p1 ? "true" : "false");
    // HomieInternals::Interface::get().getSendingPromise().setNode(presence1).setProperty("presence").setQos(2).setRetained(true).overwriteSetter(false).setRange({ .isRange = false, .index = 0 }).send(p1 ? "true" : "false");
    _presence1 = p1;
  }

  if(force || (_presence2 != p2))
  {
    presence2.setProperty("presence").send(p2 ? "true" : "false");
    // HomieInternals::Interface::get().getSendingPromise().setNode(presence2).setProperty("presence").setQos(2).setRetained(true).overwriteSetter(false).setRange({ .isRange = false, .index = 0 }).send(p2 ? "true" : "false");
    _presence2 = p2;
  }

  if(force || (_presence3 != p3))
  {
    presence3.setProperty("presence").send(p3 ? "true" : "false");
    // HomieInternals::Interface::get().getSendingPromise().setNode(presence3).setProperty("presence").setQos(2).setRetained(true).overwriteSetter(false).setRange({ .isRange = false, .index = 0 }).send(p3 ? "true" : "false");
    _presence3 = p3;
  }
}

void setupHandler()
{
  send_presence(true);
}

void loopHandler()
{
  if (millis() - lux_mark >= LUX_INTERVAL || lux_mark == 0)
  {
    int lux = analogRead(0);
    Homie.getLogger() << "lux: " << lux << endl;
    luxNode.setProperty("lux").send(String(lux));
    lux_mark = millis();
  }

  if (millis() - presence_mark >= PRESENCE_INTERVAL || presence_mark == 0)
  {
      send_presence(true);
      presence_mark = millis();
  }
  send_presence();
}

void setup()
{
  Serial.begin(115200);
  Serial << endl << endl;
  
  pinMode(D1, INPUT);
  pinMode(D2, INPUT);
  pinMode(D4, INPUT);

  // homie
  Homie.disableLogging();
  Homie_setBrand(FW_NAME);
  Homie_setFirmware(FW_NAME, FW_VERSION);
  Homie.setSetupFunction(setupHandler).setLoopFunction(loopHandler);
  Homie.setup(); 
}

void loop()
{
  Homie.loop();
}
