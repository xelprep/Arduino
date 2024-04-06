// Ran on NodeMCU ESP8266
// Heavily inspired by https://www.makerguides.com/28byj-48-stepper-motor-arduino-tutorial/
// AccelStepper reference: http://www.airspayce.com/mikem/arduino/AccelStepper/index.html

// Built-in Stepper library hates ESP8266 due to lack of yields

// Keep in mind:
// Pay attention to pin order for ULN2003: 1,3,2,4
// Best luck when setting speed after setting move target
// Run will use acceleration and apparently only uses positive speeds
// RunSpeed runs with constant speed and requires negative speeds for CCW rotation

#include <MultiStepper.h>
#include <AccelStepper.h>

#define motorPin1  14      // IN1 on the ULN2003 driver, D5 NodeMCU
#define motorPin2  12      // IN2 on the ULN2003 driver, D6 NodeMCU
#define motorPin3  13     // IN3 on the ULN2003 driver, D7 NodeMCU
#define motorPin4  15     // IN4 on the ULN2003 driver, D8 NodeMCU

#define MotorInterfaceType 8

AccelStepper stepper = AccelStepper(MotorInterfaceType, motorPin1, motorPin3, motorPin2, motorPin4);

long stepsPerRevolution = 4096;
long stepsToMove = stepsPerRevolution;

bool firstRun = true;

void setup() {
  stepper.setMaxSpeed(2000);
  Serial.begin(115200);
}

void loop() {
  if ( firstRun ) {
    Serial.println();
    Serial.println("Performing first run operation");
    firstRun = false;
    stepper.moveTo(stepsToMove);
    stepper.setSpeed(1100);
    Serial.println("Moving to " + String(stepsToMove));
  }
  else if ( abs(stepper.distanceToGo()) > 0 ) {
    //Serial.println("Distance to go: " + String(abs(stepper.distanceToGo())));
    stepper.runSpeed();
  }
  else {
    Serial.println("Changing directions!");
    Serial.println("Old target: " + String(stepsToMove));
    if ( stepsToMove == 0 ) {
      Serial.println("Setting new target to original: " + String(stepsPerRevolution));
      stepsToMove = stepsPerRevolution;
    }
    else {
      stepsToMove = 0;
    }
    stepper.moveTo(stepsToMove);
    Serial.println("New target: " + String(stepsToMove));
    if ( stepper.distanceToGo() < 0 ) {
      stepper.setSpeed(-1100);
    }
    else {
      stepper.setSpeed(1100);
    }
    Serial.println("Distance to go: " + String(stepper.distanceToGo()));
  }
}
