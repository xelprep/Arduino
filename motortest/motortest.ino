// Ran on NodeMCU ESP8266
// Heavily inspired by https://www.makerguides.com/28byj-48-stepper-motor-arduino-tutorial/

// Built-in Stepper library hates ESP8266 due to lack of yields

// This version also has WDT resets... Need to troubleshoot


#include <MultiStepper.h>
#include <AccelStepper.h>

const int stepsPerRevolution = 2048;

#define motorPin1  14      // IN1 on the ULN2003 driver, D5 NodeMCU
#define motorPin2  12      // IN2 on the ULN2003 driver, D6 NodeMCU
#define motorPin3  13     // IN3 on the ULN2003 driver, D7 NodeMCU
#define motorPin4  15     // IN4 on the ULN2003 driver, D8 NodeMCU

#define MotorInterfaceType 4

AccelStepper stepper = AccelStepper(MotorInterfaceType, motorPin1, motorPin3, motorPin2, motorPin4);

void setup() {
  // Set the maximum steps per second:
  stepper.setMaxSpeed(1000);

  Serial.begin(9600);
}

void loop() {
  // Set the current position to 0:
  stepper.setCurrentPosition(0);
  // Run the motor forward at 500 steps/second until the motor reaches 4096 steps (1 revolution):
  while (stepper.currentPosition() != 4096) {
    stepper.setSpeed(500);
    stepper.runSpeed();
  }
  delay(1000);
  // Reset the position to 0:
  stepper.setCurrentPosition(0);
  // Run the motor backwards at 1000 steps/second until the motor reaches -4096 steps (1 revolution):
  while (stepper.currentPosition() != -4096) {
    stepper.setSpeed(-1000);
    stepper.runSpeed();
  }
  delay(1000);
  // Reset the position to 0:
  stepper.setCurrentPosition(0);
  // Run the motor forward at 1000 steps/second until the motor reaches 8192 steps (2 revolutions):
  while (stepper.currentPosition() != 8192) {
    stepper.setSpeed(1000);
    stepper.runSpeed();
  }
  delay(3000);
}
