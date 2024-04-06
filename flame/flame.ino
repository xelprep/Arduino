// Basic sketch to simulate a flame from a RGB LED
// Ran on Arduino Nano

#include "SoftPWM.h"

const int greenpin = 3; // Due to simplicity of soldering, this is the only real PWM pin
const int redpin = 2; // Needs SoftPWM
const int bluepin = 4; // Needs SoftPWM

int gintensity;
int rintensity;
int bintensity;
int timer;

void setup() {
  SoftPWMBegin();
  SoftPWMSet(greenpin, 0);
  SoftPWMSet(redpin, 0);
  SoftPWMSet(bluepin, 0);
}

void loop() {

  // Generate random values for red, green, and the delay
  gintensity = random(10, 50);
  rintensity = random(180, 255);
  bintensity = random(0, 10);
  timer = random(5, 30);
  SoftPWMSet(greenpin, gintensity);
  SoftPWMSet(redpin, rintensity);
  SoftPWMSet(bluepin, bintensity);
  delay(timer);
}
