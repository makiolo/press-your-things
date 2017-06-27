#include <Homie.h>

HomieNode presence1("presence_1", "presence");
HomieNode presence2("presence_2", "presence");
HomieNode presence3("presence_3", "presence");
HomieNode luxNode("lux", "lux");

const int TEMPERATURE_INTERVAL = 3  * 1000UL;
const int PRESENCE_INTERVAL = 10;
unsigned long lastTemperatureSent = 0;
unsigned long lastPresenceSent = 0;

void loopHandler()
{
  if (millis() - lastTemperatureSent >= TEMPERATURE_INTERVAL || lastTemperatureSent == 0)
  {
    int temperature = analogRead(0);
    Homie.getLogger() << "lux: " << temperature << " °C" << endl;
    luxNode.setProperty("lux").send(String(temperature));
    lastTemperatureSent = millis();
  }

  if (millis() - lastPresenceSent >= PRESENCE_INTERVAL || lastPresenceSent == 0)
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
    
      if(_presence1 != p1)
      {
        HomieInternals::Interface::get().getSendingPromise().setNode(presence1).setProperty("presence").setQos(2).setRetained(true).overwriteSetter(false).setRange({ .isRange = false, .index = 0 }).send(p1 ? "true" : "false");
        _presence1 = p1;
      }
    
      if(_presence2 != p2)
      {
        HomieInternals::Interface::get().getSendingPromise().setNode(presence2).setProperty("presence").setQos(2).setRetained(true).overwriteSetter(false).setRange({ .isRange = false, .index = 0 }).send(p2 ? "true" : "false");
        _presence2 = p2;
      }
    
      if(_presence3 != p3)
      {
        HomieInternals::Interface::get().getSendingPromise().setNode(presence3).setProperty("presence").setQos(2).setRetained(true).overwriteSetter(false).setRange({ .isRange = false, .index = 0 }).send(p3 ? "true" : "false");
        _presence3 = p3;
      }
      lastTemperatureSent = millis();
  }

}

void setup()
{
  Serial.begin(115200);
  Serial << endl << endl;
  
  pinMode(D1, INPUT);
  pinMode(D2, INPUT);
  pinMode(D4, INPUT);

  // homie
  // Homie.disableLogging();
  Homie_setBrand("SalonDev");
  Homie_setFirmware("salon-dev", "0.0.5");
  Homie.setLoopFunction(loopHandler);
  Homie.setup(); 
}

void loop()
{
  Homie.loop();
}
