// Hell Gauntlet
// -------------
// Authored by xelprep

// Tested with the following libraries:
// ESP32 BLE Keyboard@0.3.2 - with NimBLE mode enabled (see docs for library)
// NimBLE-Arduino@1.4.1
// ToneESP32@1.0.0
// FastLED@3.6.0

// Should only compile for a WEMOS LOLIN32, Adafruit ESP32 Feather V2, or compatible clones.
// Should fail to compile for other boards

// TODO:
// Implement dual core for ensuring priority of keyboard responsiveness
// Implement the feature where a pin is low and the chip is turned off but power stuff keeps going
// Add LEDs and effects based on connection and power statuses
// Test on a ps5
// Test if HD respects inputs from multiple keyboards

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

bool jinglePlayed = false;
int batteryReportCounter = 0;
int Vbatt = 0;
float Vbattnormalized = 0;
float Vbattpercent = 0;
byte previousButtonStates[numOfButtons];
byte currentButtonStates[numOfButtons];
byte buttonPins[numOfButtons] = { BUTTON1, BUTTON2, BUTTON3, BUTTON4, BUTTON5 };
byte physicalButtons[numOfButtons] = { KEY_LEFT_CTRL, 'w', 'a', 's', 'd' };

CRGB leds[NUM_LEDS];
BleKeyboard bleKeyboard("Hell Gauntlet", "S.E.A.F Armory", 100);
ToneESP32 buzzer(BUZZER_PIN, BUZZER_CHANNEL);

void setup() {
  initLED();
  initPins();
  bleKeyboard.begin();
}

void loop() {
  if (!bleKeyboard.isConnected()) {
    jinglePlayed = false;
  }

  if (bleKeyboard.isConnected()) {
    if (!jinglePlayed) {
      delay(2000);
      helldive();
    }

    if (batteryReportCounter == 500) {  // Limit the number of battery reports to 1 per 10 seconds 500 counts * 20 millis = 10000 millis
      for (int i = 0; i < 51; i++) {
        if (i != 0) {                                         // Internet says discard first reading since ESP ADC sucks
          Vbatt = Vbatt + analogReadMilliVolts(BATTERY_PIN);  // ADC with correction
        }
      }
      float Vbattf = 2 * Vbatt / 50 / 1000.0;  // attenuation ratio 1/2, mV --> V

      Vbattnormalized = Vbattf - 3.3;  // Subract safe voltage

      if (Vbattnormalized < 0) {
        Vbattnormalized = 0;  // Sometimes it's too low so let's pretend it's dead
      }

      if (Vbattnormalized > .9) {
        Vbattnormalized = .9;  // Sometimes it's too high. Hopefully there's a box of sand nearby
      }

      Vbattpercent = Vbattnormalized / .9 * 100;

      Vbattpercent = round(Vbattpercent);

      bleKeyboard.setBatteryLevel(Vbattpercent);
      batteryReportCounter = 0;
    } else {
      batteryReportCounter = batteryReportCounter + 1;
    }

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

void helldive() {
  leds[0] = CRGB::Red;
  FastLED.show();
  buzzer.tone(NOTE_F5, 200);
  delay(20);
  leds[0] = CRGB::White;
  FastLED.show();
  buzzer.tone(NOTE_E5, 200);
  delay(20);
  leds[0] = CRGB::Blue;
  FastLED.show();
  buzzer.tone(NOTE_D5, 200);
  delay(20);
  leds[0] = CRGB::OrangeRed;
  FastLED.show();
  buzzer.tone(NOTE_A4, 1500);
  delay(20);
  leds[0] = CRGB::Green;
  FastLED.show();
  buzzer.tone(NOTE_C5, 800);
  delay(20);
  leds[0] = CRGB::Yellow;
  FastLED.show();
  buzzer.tone(NOTE_D5, 1500);
  jinglePlayed = true;
}

void initLED() {
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(10);
  delay(1000);
  leds[0] = CRGB::Green;
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