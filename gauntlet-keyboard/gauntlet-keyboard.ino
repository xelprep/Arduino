/*
Hell Gauntlet
-------------
Authored by xelprep cobbled together from various example code in the used libraries

Tested with the following libraries:
ESP32 BLE Keyboard@0.3.2 - with NimBLE mode enabled (see docs for library)
NimBLE-Arduino@1.4.1
ToneESP32@1.0.0
FastLED@3.6.0

Tested with version 2.0.14 ESP32 Arduino Core

Should only compile for a WEMOS LOLIN32, Adafruit ESP32 Feather V2, or compatible clones.
Should fail to compile for other boards

TODO:
Test higher brightness on LEDs to get an actual color gradient as state of charge changes
Add fast blink when battery is critically low
Implement deep sleep when battery is too low
Test on a ps5
Test if HD respects inputs from multiple keyboards
Maybe move jingle back to high-priority loop to prevent stuttering
Change jingle colors to match Super Earth flag colors
*/

#include <BleKeyboard.h>
#include <ToneESP32.h>
#include <FastLED.h>

#ifdef ARDUINO_LOLIN32
#define BATTERY_PIN 4
#define BUZZER_PIN 22
#define LED_PIN 13
#define BUTTON1 12
#define BUTTON2 16
#define BUTTON3 17
#define BUTTON4 18
#define BUTTON5 19
#define BOARD "LOLIN32"
#endif

#ifdef ARDUINO_ADAFRUIT_FEATHER_ESP32_V2
#define BATTERY_PIN 4
#define BUZZER_PIN 22
#define LED_PIN 13
#define BUTTON1 12
#define BUTTON2 16
#define BUTTON3 17
#define BUTTON4 18
#define BUTTON5 19
#define BOARD "FEATHERV2"
#endif

#ifndef BOARD
#error "No compatible board defined"
#endif

const int numOfButtons = 5;
const int BUZZER_CHANNEL = 0;
const int NUM_LEDS = 1;
const long interval = 500;

bool jinglePlayed = false;
bool ledState = true;
int batteryReportCounter = 0;
int Vbatt = 0;
int fakeBatt = 0;
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
float Vbattnormalized = 0;
float Vbattpercent = 0;
byte previousButtonStates[numOfButtons];
byte currentButtonStates[numOfButtons];
byte buttonPins[numOfButtons] = { BUTTON1, BUTTON2, BUTTON3, BUTTON4, BUTTON5 };
byte physicalButtons[numOfButtons] = { KEY_LEFT_CTRL, 'w', 'a', 's', 'd' };

TaskHandle_t loopLowPri;  // Instantiate another task to run on low-priority core 0

CRGB leds[NUM_LEDS];
BleKeyboard bleKeyboard("Hell Gauntlet", "S.E.A.F Armory", 100);
ToneESP32 buzzer(BUZZER_PIN, BUZZER_CHANNEL);

void setup() {
  initLED();
  initPins();
  bleKeyboard.begin();

  batteryReportCounter = 1500;  // Ensure battery status is updated post-jingle so we're not hanging on yellow

  delay(500);

  xTaskCreatePinnedToCore(  // This is where we define the function to run as the low priotity task
    loopLowPriCode,         /* Function to implement the task */
    "loopLowPri",           /* Name of the task */
    10000,                  /* Stack size in words */
    NULL,                   /* Task input parameter */
    0,                      /* Priority of the task */
    &loopLowPri,            /* Task handle. */
    0);                     /* Core where the task should run */
}

void loop() {
  if (bleKeyboard.isConnected()) {
    for (byte currentIndex = 0; currentIndex < numOfButtons; currentIndex++) {
      currentButtonStates[currentIndex] = digitalRead(buttonPins[currentIndex]);

      if (currentButtonStates[currentIndex] != previousButtonStates[currentIndex]) {
        if (currentButtonStates[currentIndex] == LOW) {
          bleKeyboard.press(physicalButtons[currentIndex]);
        } else {
          bleKeyboard.release(physicalButtons[currentIndex]);
        }
      }
    }

    if (currentButtonStates != previousButtonStates) {
      for (byte currentIndex = 0; currentIndex < numOfButtons; currentIndex++) {
        previousButtonStates[currentIndex] = currentButtonStates[currentIndex];
      }
    }

    delay(20);
  }
}

void loopLowPriCode(void* parameter) {
  for (;;) {
    //fakeBattery();
    batteryStatus();
    if (!bleKeyboard.isConnected()) {
      currentMillis = millis();
      if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        if (ledState) {
          ledState = false;
          leds[0] = CHSV(int(Vbattpercent), 255, 0);
        } else {
          ledState = true;
          leds[0] = CHSV(int(Vbattpercent), 255, 10);
        }
        FastLED.show();  // Update LED to represent current state of charge
      }
      jinglePlayed = false;
    }

    if (bleKeyboard.isConnected()) {
      if (!jinglePlayed) {
        delay(1000);
        helldive();
        batteryReportCounter = 1500;  // Ensure battery status is updated post-jingle so we're not hanging on yellow
      }
      leds[0] = CHSV(int(Vbattpercent), 255, 10);
      FastLED.show();  // Update LED to represent current state of charge
    }
  }
}

void helldive() {
  leds[0] = CHSV(0, 255, 10);  //CRGB::Red;
  FastLED.show();
  buzzer.tone(NOTE_F5, 200);
  delay(20);
  leds[0] = CHSV(96, 0, 10);  //CRGB::White;
  FastLED.show();
  buzzer.tone(NOTE_E5, 200);
  delay(20);
  leds[0] = CHSV(160, 255, 10);  //CRGB::Blue;
  FastLED.show();
  buzzer.tone(NOTE_D5, 200);
  delay(20);
  leds[0] = CHSV(44, 220, 10);  //CRGB::OrangeRed;
  FastLED.show();
  buzzer.tone(NOTE_A4, 1500);
  delay(20);
  leds[0] = CHSV(96, 255, 10);  //CRGB::Green;
  FastLED.show();
  buzzer.tone(NOTE_C5, 800);
  delay(20);
  leds[0] = CHSV(64, 255, 10);  //CRGB::Yellow;
  FastLED.show();
  buzzer.tone(NOTE_D5, 1500);
  jinglePlayed = true;
}

void initLED() {
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  //FastLED.setBrightness(10);
  delay(1000);
  leds[0] = CHSV(96, 255, 10);
  FastLED.show();
}

void initPins() {
  pinMode(BATTERY_PIN, INPUT_PULLDOWN);  // ADC
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);  // Turn LED on so you know it's on when unplugged

  for (byte currentPinIndex = 0; currentPinIndex < numOfButtons; currentPinIndex++) {
    pinMode(buttonPins[currentPinIndex], INPUT_PULLUP);
    previousButtonStates[currentPinIndex] = HIGH;
    currentButtonStates[currentPinIndex] = HIGH;
  }
}

void fakeBattery() {  // Needed a way to test LED conditions
  if (batteryReportCounter == 1500) {
    batteryReportCounter = 0;
  }
  if (batteryReportCounter == 50) {
    Vbattpercent = fakeBatt;
    bleKeyboard.setBatteryLevel(int(Vbattpercent));
    if (fakeBatt == 100) {
      fakeBatt = 0;
    } else {
      fakeBatt = fakeBatt + 1;
    }
    batteryReportCounter = 0;
  } else {
    batteryReportCounter = batteryReportCounter + 1;
  }
  delay(20);
}

void batteryStatus() {
  if (batteryReportCounter == 1500) {  // Limit the number of battery reports to ~1 per 30 seconds 1500 counts * 20 millis = 30000 millis
    for (int i = 0; i < 10001; i++) {
      if (i != 0) {  // Internet says discard first reading since ESP ADC sucks
        Vbatt = Vbatt + analogReadMilliVolts(BATTERY_PIN);
      }
    }
    float Vbattf = 2 * Vbatt / 10000 / 1000.0;  // attenuation ratio 1/2, 10000 readings, mV --> V

    Vbattnormalized = Vbattf - 3.3;  // Subract safe voltage. 3.3 is the lowest value I'm comfortable going. 4.2 is the max. 4.2 - 3.3 = .9

    if (Vbattnormalized < 0) {
      Vbattnormalized = 0;  // Sometimes it's too low so let's pretend it's dead
    }

    if (Vbattnormalized > .9) {
      Vbattnormalized = .9;  // Sometimes it's too high. Hopefully there's a box of sand nearby
    }

    Vbattpercent = Vbattnormalized / .9 * 100;

    Vbattpercent = round(Vbattpercent);

    bleKeyboard.setBatteryLevel(int(Vbattpercent));
    batteryReportCounter = 0;
  } else {
    batteryReportCounter = batteryReportCounter + 1;
  }
  delay(20);
}