// TODO:
// Implement dual core for ensuring priority of keyboard responsiveness
// Implement the feature where a pin is low and the chip is turned off but power stuff keeps going
// Disable Serial output for performance reasons maybe?
// Change defines to consts
// Add LEDs and effects based on connection and power statuses
// Test on a ps5
// Test if HD respects inputs from multiple keyboards

#include <BleKeyboard.h>
#include <ToneESP32.h>

BleKeyboard bleKeyboard("Hell Gauntlet", "Linguanal Sense", 100);

#define numOfButtons 5
#define BATTERY_PIN 4
#define BUZZER_PIN 22
#define BUZZER_CHANNEL 0

uint32_t batteryReportCounter = 0;
bool jinglePlayed = false;

byte previousButtonStates[numOfButtons];
byte currentButtonStates[numOfButtons];
byte buttonPins[numOfButtons] = { 3, 16, 17, 18, 19 };
byte physicalButtons[numOfButtons] = { KEY_LEFT_CTRL, 'w', 'a', 's', 'd' };

ToneESP32 buzzer(BUZZER_PIN, BUZZER_CHANNEL);

void helldive() {
  buzzer.tone(NOTE_F5, 200);
  delay(20);
  buzzer.tone(NOTE_E5, 200);
  delay(20);
  buzzer.tone(NOTE_D5, 200);
  delay(20);
  buzzer.tone(NOTE_A4, 1500);
  delay(20);
  buzzer.tone(NOTE_C5, 800);
  delay(20);
  buzzer.tone(NOTE_D5, 1500);
  buzzer.noTone();
}

void setup() {

  // Serial.begin(115200);
  pinMode(BATTERY_PIN, INPUT_PULLDOWN);  // ADC
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);  // Turn LED on so you know it's on when unplugged

  for (byte currentPinIndex = 0; currentPinIndex < numOfButtons; currentPinIndex++) {
    pinMode(buttonPins[currentPinIndex], INPUT_PULLUP);
    previousButtonStates[currentPinIndex] = HIGH;
    currentButtonStates[currentPinIndex] = HIGH;
  }

  bleKeyboard.begin();
}

void loop() {
  if (bleKeyboard.isConnected()) {
    float Vbattnormalized = 0;
    float Vbattpercent = 0;
    uint32_t Vbatt = 0;

    if (jinglePlayed != true) {
      helldive();
      jinglePlayed = true;
    }

    if (batteryReportCounter == 500) {  // Limit the number of battery reports to 1 per 10 seconds 500*20 = 10000
      for (int i = 0; i < 50; i++) {
        Vbatt = Vbatt + analogReadMilliVolts(BATTERY_PIN);  // ADC with correction
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

      // Serial.print(F("Battery voltage is approximately "));
      // Serial.println(Vbattf, 3);
      // Serial.print(F("Battery percentage is "));
      // Serial.print(Vbattpercent, 3);
      // Serial.println("%");

      bleKeyboard.setBatteryLevel(round(Vbattpercent));
      batteryReportCounter = 0;
    } else {
      batteryReportCounter = batteryReportCounter + 1;
    }

    // Serial.println(batteryReportCounter, 3);

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
  } else {
    jinglePlayed = false;
  }
}
