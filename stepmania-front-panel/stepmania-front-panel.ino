// Tested this using a Pro Micro 5v clone from Aliexpress
// Based on an example from https://github.com/MHeironimus/ArduinoJoystickLibrary
#include <Joystick.h>

// Construct Joystick
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD,
                   8, 0,                  // Button Count, Hat Switch Count
                   false, false, false,   // No Axes, only buttons
                   false, false, false,   // No Rx, Ry, or Rz
                   false, false,          // No rudder or throttle
                   false, false, false);  // No accelerator, brake, or steering

// Define LED pin numbers because GPIO numbering on the Pro Micro is insane
const int led1 = 21;
const int led2 = 20;
const int led3 = 19;
const int led4 = 18;
const int led5 = 15;
const int led6 = 14;
const int led7 = 16;
const int led8 = 10;

unsigned long startMillis;
unsigned long currentMillis;
const int period = 250;
int ledFun = 0;

void setup() {
  // Button pins
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);

  // Button LED pins
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  pinMode(led5, OUTPUT);
  pinMode(led6, OUTPUT);
  pinMode(led7, OUTPUT);
  pinMode(led8, OUTPUT);

  Joystick.begin();
}

// Last state of the buttons
int lastButtonState[8] = {0, 0, 0, 0, 0, 0, 0, 0};

void loop() {
  // Read pin values
  for (int index = 0; index < 8; index++) {
    int currentButtonState = !digitalRead(index + 2);
    if (currentButtonState != lastButtonState[index]) {
      Joystick.setButton(index, currentButtonState);
    }
    lastButtonState[index] = currentButtonState;
  }

  ledStuff();

  delay(10);
}

void ledStuff() {
  currentMillis = millis();

  // LED Stuff
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
