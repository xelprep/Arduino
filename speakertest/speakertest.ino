// Ran on NodeMCU ESP8266
// Outputs audio to RX pin and requires ESP8266Audio and ESP8266SAM submodules

#include <Arduino.h>
#include <ESP8266SAM.h>
#include <AudioOutputI2SNoDAC.h>

AudioOutputI2SNoDAC *out = NULL;

void setup()
{
  out = new AudioOutputI2SNoDAC();
  out->begin();
}

void loop()
{
  ESP8266SAM *sam = new ESP8266SAM;
  //sam->SetVoice(ESP8266SAM::VOICE_SAM);
  //sam->SetSpeed(150);
  sam->Say(out, "Test text goes here");
  delete sam;
  delay(1000);
}
