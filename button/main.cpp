#include <Arduino.h>
#include <Homie.h>
#include <ArduinoOTA.h>
#define FW_NAME "button"
#define FW_VERSION "1.0.0"
const char *__FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" FW_NAME "\x93\x44\x6b\xa7\x75";
const char *__FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" FW_VERSION "\xb0\x30\x48\xd4\x1a";

#if 0
const int PIN_RELAY = 12;
const bool inverse_logic_relay = false;
#else
const int PIN_RELAY = 2;
const bool inverse_logic_relay = false;
#endif

const int PIN_BUTTON = 0;
const int PIN_LED = 13;

unsigned long buttonDownTime = 0;
byte buttonPressHandled = 0;
byte lastButtonState = 1;
unsigned long lastClickTime = 0;
float catchTime = 0.5;

HomieNode buttonNode("button", "button");

// enviar estado a topic: homie/rele/button/on/set
bool switchOnHandler(HomieRange range, String value)
{
  if (value == "toggle")
  {
    toggleRelay();
    return true;
  }
  else if (value == "true")
  {
    press_on();
    return true;
  }
  else if (value == "false")
  {
    press_off();
    return true;
  }
  return false;
}

void press_on() {
  _press(true);
}

void press_off() {
  _press(false);
}

void toggleRelay()
{
  if(inverse_logic_relay)
  {
    _press(digitalRead(PIN_RELAY) == HIGH);
  }
  else
  {
    _press(digitalRead(PIN_RELAY) == LOW);
  }
}

void _press(bool on)
{
  if(inverse_logic_relay)
  {
    digitalWrite(PIN_RELAY, on ? LOW : HIGH);
  }
  else
  {
    digitalWrite(PIN_RELAY, on ? HIGH : LOW);
  }
  buttonNode.setProperty("on").send(on ? "true" : "false");
  Homie.getLogger() << "Switch is " << (on ? "on" : "off") << endl;
}

void loopHandler_new()
{
  byte buttonState = digitalRead(PIN_BUTTON);
  if ( buttonState != lastButtonState )
  {
    if (buttonState == LOW)
    {
      unsigned long now = millis();
      if(float(now - lastClickTime) < catchTime)
      {
        // TODO: dont work:
        // idea is detect double click
        Homie.getLogger() << "double click" << endl;
        press_on();
      }
      else
      {
        // normal click
        Homie.getLogger() << "normal click" << endl;
        toggleRelay();
      }
      lastClickTime = now;
    }
    lastButtonState = buttonState;
  }
}

void loopHandler()
{
  byte buttonState = digitalRead(PIN_BUTTON);
  if ( buttonState != lastButtonState ) {
    if (buttonState == LOW) {
      buttonDownTime     = millis();
      buttonPressHandled = 0;
    }
    else {
      unsigned long dt = millis() - buttonDownTime;
      if ( dt >= 90 && dt <= 900 && buttonPressHandled == 0 ) {
        toggleRelay();
        buttonPressHandled = 1;
      }
    }
    lastButtonState = buttonState;
  }
}

void setup()
{
  Serial.begin(115200);
  Serial << endl << endl;

  // rele
  pinMode(PIN_BUTTON, INPUT);
  pinMode(PIN_RELAY, OUTPUT);

  // homie
  //Homie.setLedPin(PIN_LED, LOW).setResetTrigger(PIN_BUTTON, LOW, 5000);
  Homie.disableLedFeedback();
  //Homie.disableLogging();
  Homie_setBrand(FW_NAME);
  Homie_setFirmware(FW_NAME, FW_VERSION);
  Homie.setLoopFunction(loopHandler);
  buttonNode.advertise("on").settable(switchOnHandler);
  Homie.setup();
  ArduinoOTA.setHostname(Homie.getConfiguration().deviceId);
  ArduinoOTA.begin();
  // press_on();
}

void loop()
{
  Homie.loop();
  ArduinoOTA.handle();
}
