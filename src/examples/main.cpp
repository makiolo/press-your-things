#define FW_NAME "main"
#define FW_VERSION "1.0.0"

#include <Homie.h>

void setup()
{
  Serial.begin(115200);
  Serial << endl << endl;

  Homie_setFirmware(FW_NAME, FW_VERSION);
  Homie.setSetupFunction(setupHandler);
  Homie.setup();
}

void loop()
{
  Homie.loop();
}
