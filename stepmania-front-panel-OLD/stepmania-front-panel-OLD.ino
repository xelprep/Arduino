
#include "UnoJoy.h"

// Define our pins and variables

unsigned long startMillis;
unsigned long currentMillis;
const unsigned long period = 250;

int TrianglePin = 4;
int SquarePin = 5;
int CrossPin = 3;
int CirclePin = 2;

int l1Pin = 6;
int l2Pin = 7;
int r1Pin = 8;
int r2Pin = 9;

int led1 = 10;
int led2 = 11;
int led3 = 12;
int led4 = 13;
int led5 = 14;
int led6 = 15;
int led7 = 16;
int led8 = 17;

int ledFun = 0;

void setup(){
  setupPins();
  setupUnoJoy();
  startMillis = millis();
}

void loop(){
  // Always be getting fresh data
  dataForController_t controllerData = getControllerData();
  setControllerData(controllerData);

  // LED fun
  currentMillis = millis();
  
  if (currentMillis - startMillis >= period)
  {
    switch (ledFun) {
      case 0:
          digitalWrite(led1, HIGH);
          digitalWrite(led2, LOW);
          digitalWrite(led3, LOW);
          digitalWrite(led4, LOW);
          digitalWrite(led5, HIGH);
          digitalWrite(led6, LOW);
          digitalWrite(led7, LOW);
          digitalWrite(led8, LOW);
          startMillis = currentMillis;
          ledFun = 1;
          break;
      case 1:
          digitalWrite(led1, LOW);
          digitalWrite(led2, HIGH);
          digitalWrite(led3, LOW);
          digitalWrite(led4, LOW);
          digitalWrite(led5, LOW);
          digitalWrite(led6, HIGH);
          digitalWrite(led7, LOW);
          digitalWrite(led8, LOW);
          startMillis = currentMillis;
          ledFun = 2;
          break;
      case 2:
          digitalWrite(led1, LOW);
          digitalWrite(led2, LOW);
          digitalWrite(led3, HIGH);
          digitalWrite(led4, LOW);
          digitalWrite(led5, LOW);
          digitalWrite(led6, LOW);
          digitalWrite(led7, HIGH);
          digitalWrite(led8, LOW);
          startMillis = currentMillis;
          ledFun = 3;
          break;
      case 3:
          digitalWrite(led1, LOW);
          digitalWrite(led2, LOW);
          digitalWrite(led3, LOW);
          digitalWrite(led4, HIGH);
          digitalWrite(led5, LOW);
          digitalWrite(led6, LOW);
          digitalWrite(led7, LOW);
          digitalWrite(led8, HIGH);
          startMillis = currentMillis;
          ledFun = 0;
          break;
    }
  }
}

void setupPins(void){
  // Set all the digital pins as inputs
  // with the pull-up enabled, except for the 
  // two serial line pins
  for (int i = 2; i <= 9; i++){
    pinMode(i, INPUT_PULLUP);
    digitalWrite(i, HIGH);
  }
  for (int i = 10; i <= 17; i++){
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }
  digitalWrite(10, HIGH);
  digitalWrite(14, HIGH);
}

dataForController_t getControllerData(void){
  
  // Set up a place for our controller data
  //  Use the getBlankDataForController() function, since
  //  just declaring a fresh dataForController_t tends
  //  to get you one filled with junk from other, random
  //  values that were in those memory locations before
  dataForController_t controllerData = getBlankDataForController();
  // Since our buttons are all held high and
  //  pulled low when pressed, we use the "!"
  //  operator to invert the readings from the pins
  controllerData.triangleOn = !digitalRead(TrianglePin);
  controllerData.circleOn = !digitalRead(CirclePin);
  controllerData.squareOn = !digitalRead(SquarePin);
  controllerData.crossOn = !digitalRead(CrossPin);
  controllerData.l1On = !digitalRead(l1Pin);
  controllerData.l2On = !digitalRead(l2Pin);
  controllerData.r1On = !digitalRead(r1Pin);
  controllerData.r2On = !digitalRead(r2Pin);
  
  // And return the data!
  return controllerData;
}
