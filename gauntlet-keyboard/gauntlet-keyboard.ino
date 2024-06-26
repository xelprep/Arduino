/*
Hell Gauntlet
-------------
Authored by xelprep cobbled together from various example code in the used libraries

Tested with the following libraries:
ESP32 BLE Keyboard@0.3.2 - with NimBLE mode enabled and some additional security modifications done - see libraries folder
NimBLE-Arduino@1.4.1
ToneESP32@1.0.0
FastLED@3.6.0
AsyncTCP@1.1.4
ESPAsyncWebServer@3.1.0

Tested with version 2.0.15 ESP32 Arduino Core

Should only compile for a WEMOS LOLIN32, Adafruit ESP32 Feather V2, or compatible clones.
Should fail to compile for other boards

TODO:
Test on a ps5
Test if HD respects inputs from multiple keyboards

NOTES
Set batteryTestMode to true if you want to run battery test with fake battery status.
Starts at 100 and decreases 1% every ~1 second.

Setting batteryTestMode to false reads actual connected battery. If no battery is inserted, at least on the LOLIN32 clone, 
ADC reads full battery due to being connected to charging module. Unsure how to fix or if it's even possible.

Set ledBright to an integer from 0-255, defaults to 10 but might need to go higher if using the onboard neopixel
Keep as low as possible since we're technically running the WS2812B out of spec at 3.3v

Captive portal works, but only for http. Https requests will hang. Link to http://anything.tld (NO 'S') and it should work.

Uncomment PS5 definition to invoke all the wifi and auth stuff for that platform
*/

// Platform definition
#define PS5

#ifdef PS5
#include <BleKeyboardWifi.h>
#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#else
#include <BleKeyboardJW.h>
#endif

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
#define BATTERY_PIN BATT_MONITOR
#define BUZZER_PIN 27
#define LED_PIN 0  // Built-in Neopixel
#define BUTTON1 15
#define BUTTON2 32
#define BUTTON3 14
#define BUTTON4 20
#define BUTTON5 22
#define BOARD "FEATHERV2"
#endif

#ifndef BOARD
#error "No compatible board defined"
#endif

const int numOfButtons = 5;
const int BUZZER_CHANNEL = 0;
const int NUM_LEDS = 1;
const int ledBright = 60;
const long dcinterval = 500;     // LED blink interval when BT is not connected - 500ms
const long btinterval = 20;      // Using millis to unblock core0 instead of 20ms delay()
const long lobatinterval = 125;  // LED blink interval when battery is low - 125ms
const long provinterval = 30;    // LED blink interval during wifi provisioning - 30ms
const char* ssid = "SUPEREARTH";
//const char* password = "12345678";
const char* PARAM_INPUT_1 = "input1";
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>Bluetooth Passkey</title><h2>Input Your Bluetooth Pairing Passkey</h2>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <form action="/get">
    Passkey: <input type="number" name="input1">
    <input type="submit" value="Submit">
  </form><br>Make sure to load this page before starting the pairing process because 
  once the pairing process starts, you only have 30 seconds to submit before 
  you have to start over.
</body></html>)rawliteral";

bool batteryTestMode = false;
bool jinglePlayed = false;
bool ledState = true;
bool wifiEnabled = false;
int batteryReportCounter = 0;
int fakeBatt = 100;
int batteryHue = 80;
int zeroBattTimes = 0;
int Vbattpercent = 100;
extern uint32_t passKey;
unsigned long dcpreviousMillis = 0;
unsigned long dccurrentMillis = 0;
unsigned long btpreviousMillis = 0;
unsigned long btcurrentMillis = 0;
unsigned long lobatpreviousMillis = 0;
unsigned long lobatcurrentMillis = 0;
unsigned long provpreviousMillis = 0;
unsigned long provcurrentMillis = 0;
float Vbatt = 0;
float analogBatt = 0;
float Vbattf = 0;
float Vbattnormalized = 0;
byte previousButtonStates[numOfButtons];
byte currentButtonStates[numOfButtons];
byte buttonPins[numOfButtons] = { BUTTON1, BUTTON2, BUTTON3, BUTTON4, BUTTON5 };

#ifdef PS5
byte physicalButtons[numOfButtons] = { KEY_LEFT_CTRL, 'w', 'a', 's', 'd' };
#else
byte physicalButtons[numOfButtons] = { KEY_LEFT_CTRL, 'i', 'j', 'k', 'l' };
#endif

TaskHandle_t loopCore0task;  // Instantiate another task to run on low-priority core 0

#ifdef PS5
AsyncWebServer server(80);
DNSServer dnsServer;
class CaptiveRequestHandler : public AsyncWebHandler {
public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest* request) {
    //request->addInterestingHeader("ANY");
    return true;
  }

  void handleRequest(AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", index_html);
  }
};
#endif

CRGB leds[NUM_LEDS];
BleKeyboard bleKeyboard("Orbital Bracer", "S.E.A.F Armory", 100);
ToneESP32 buzzer(BUZZER_PIN, BUZZER_CHANNEL);

void setup() {
  initLED();
  initPins();

#ifdef PS5
  if (digitalRead(buttonPins[0]) == LOW) {  // Hold CTRL key at boot to enter wifi provisioning mode
    wifiEnabled = true;
    leds[0] = CHSV(224, 220, ledBright);
    FastLED.show();
    delay(2000);
    WiFi.softAP(ssid);
    dnsServer.start(53, "*", WiFi.softAPIP());
    wifiStuff();
  }
#endif

  bleKeyboard.begin();

  batteryReportCounter = 1500;  // Ensure battery status is updated asap after boot

  delay(500);

  xTaskCreatePinnedToCore(  // This is where we define the function to run as the low priotity task
    loopCore0,              /* Function to implement the task */
    "loopCore0",            /* Name of the task */
    10000,                  /* Stack size in words */
    NULL,                   /* Task input parameter */
    0,                      /* Priority of the task */
    &loopCore0task,         /* Task handle. */
    0);                     /* Core where the task should run */
}

void loop() {          // Runs on core1
  if (!wifiEnabled) {  // Ignore battery status and jingle stuff during wifi provisioning
    if (batteryTestMode) {
      fakeBattery();
    } else {
      batteryStatus();
    }

    if (!bleKeyboard.isConnected()) {
      if (Vbattpercent <= 5) {  // Blink rapidly while connected to indicate low battery
        lobatcurrentMillis = millis();
        if (lobatcurrentMillis - lobatpreviousMillis >= lobatinterval) {
          lobatpreviousMillis = lobatcurrentMillis;
          if (ledState) {
            ledState = false;
            leds[0] = CHSV(batteryHue, 220, 0);
          } else {
            ledState = true;
            leds[0] = CHSV(batteryHue, 220, ledBright);
          }
          FastLED.show();  // Update LED to represent current state of charge
        }
      } else {
        dccurrentMillis = millis();
        if (dccurrentMillis - dcpreviousMillis >= dcinterval) {
          dcpreviousMillis = dccurrentMillis;
          if (ledState) {
            ledState = false;
            leds[0] = CHSV(batteryHue, 220, 0);
          } else {
            ledState = true;
            leds[0] = CHSV(batteryHue, 220, ledBright);
          }
          FastLED.show();  // Update LED to represent current state of charge
        }
      }

      jinglePlayed = false;
    }

    if (bleKeyboard.isConnected()) {
      if (!jinglePlayed) {
        delay(1000);
        helldive();
        batteryReportCounter = 1500;  // Ensure battery status is updated post-jingle so we're not hanging on the last color
      }

      if (Vbattpercent <= 5) {  // Blink rapidly while connected to indicate low battery
        lobatcurrentMillis = millis();
        if (lobatcurrentMillis - lobatpreviousMillis >= lobatinterval) {
          lobatpreviousMillis = lobatcurrentMillis;
          if (ledState) {
            ledState = false;
            leds[0] = CHSV(batteryHue, 220, 0);
          } else {
            ledState = true;
            leds[0] = CHSV(batteryHue, 220, ledBright);
          }
          FastLED.show();  // Update LED to represent current state of charge
        }
      } else {
        leds[0] = CHSV(batteryHue, 220, ledBright);
        FastLED.show();  // Update LED to represent current state of charge
      }
    }
  } else {  // Fast blink the pink during provisioning
    provcurrentMillis = millis();
    if (provcurrentMillis - provpreviousMillis >= provinterval) {
      provpreviousMillis = provcurrentMillis;
      if (ledState) {
        ledState = false;
        leds[0] = CHSV(224, 220, 0);
      } else {
        ledState = true;
        leds[0] = CHSV(224, 220, ledBright);
      }
      FastLED.show();  // Update LED to represent current state of charge
    }
#ifdef PS5
    dnsServer.processNextRequest();
#endif
  }
}

void loopCore0(void* parameter) {
  for (;;) {
    if (bleKeyboard.isConnected()) {
      btcurrentMillis = millis();

      if (btcurrentMillis - btpreviousMillis >= btinterval) {
        btpreviousMillis = btcurrentMillis;

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
      }

      if (currentButtonStates != previousButtonStates) {
        for (byte currentIndex = 0; currentIndex < numOfButtons; currentIndex++) {
          previousButtonStates[currentIndex] = currentButtonStates[currentIndex];
        }
      }
    }
  }
}

void helldive() {
  leds[0] = CHSV(64, 255, ledBright);  //CRGB::Yellow;
  FastLED.show();
  buzzer.tone(NOTE_F5, 200);
  delay(20);
  leds[0] = CHSV(64, 255, ledBright);  //CRGB::Yellow;
  FastLED.show();
  buzzer.tone(NOTE_E5, 200);
  delay(20);
  leds[0] = CHSV(64, 255, ledBright);  //CRGB::Yellow;
  FastLED.show();
  buzzer.tone(NOTE_D5, 200);
  delay(20);
  leds[0] = CHSV(160, 255, ledBright);  //CRGB::Blue;
  FastLED.show();
  buzzer.tone(NOTE_A4, 1500);
  delay(20);
  leds[0] = CHSV(96, 0, ledBright);  //CRGB::White;
  FastLED.show();
  buzzer.tone(NOTE_C5, 800);
  delay(20);
  leds[0] = CHSV(96, 0, ledBright);  //CRGB::White;
  FastLED.show();
  buzzer.tone(NOTE_D5, 1500);
  jinglePlayed = true;
}

void helldead() {
  buzzer.tone(NOTE_AS4, 200);
  delay(10);
  buzzer.tone(NOTE_AS4, 120);
  delay(10);
  buzzer.tone(NOTE_AS4, 80);
  delay(10);
  buzzer.tone(NOTE_AS4, 200);
  delay(10);
  buzzer.tone(NOTE_CS5, 120);
  delay(10);
  buzzer.tone(NOTE_C5, 80);
  delay(10);
  buzzer.tone(NOTE_C5, 120);
  delay(10);
  buzzer.tone(NOTE_AS4, 80);
  delay(10);
  buzzer.tone(NOTE_AS4, 120);
  delay(10);
  buzzer.tone(NOTE_A4, 80);
  delay(10);
  buzzer.tone(NOTE_AS4, 300);
}

void initLED() {
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  delay(1000);
  leds[0] = CHSV(160, 255, ledBright);
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

void batteryStatus() {
  if (batteryReportCounter == 1500) {  // Limit the number of battery reports to ~1 per 30 seconds 1500 counts * 20 millis = 30000 millis
    Vbatt = 0;
    for (int i = 0; i < 10001; i++) {
      if (i != 0) {  // Internet says discard first reading since ESP ADC sucks
        analogBatt = analogReadMilliVolts(BATTERY_PIN);
        if (analogBatt <= 0) {
          analogBatt = 0;
        }
        Vbatt = Vbatt + analogBatt;
      }
    }
    Vbattf = 2 * Vbatt / 10000 / 1000.0;  // attenuation ratio 1/2, 10000 readings, mV --> V

    Vbattnormalized = Vbattf - 3.3;  // Subract safe voltage. 3.3 is the lowest value I'm comfortable going. 4.2 is the max. 4.2 - 3.3 = .9

    if (Vbattnormalized < 0) {
      Vbattnormalized = 0;  // Sometimes it's too low so let's pretend it's dead
    }

    if (Vbattnormalized > .9) {
      Vbattnormalized = .9;  // Sometimes it's too high. Hopefully there's a box of sand nearby
    }

    Vbattpercent = int(round(Vbattnormalized / .9 * 100));

    bleKeyboard.setBatteryLevel(Vbattpercent);
    batteryReportCounter = 0;

    switch (Vbattpercent) {
      case 0:  // Battery is dead - make sure first and then kill the lights and sleep
        zeroBattTimes = zeroBattTimes + 1;
        if (zeroBattTimes == 3) {
          zeroBattTimes = 0;  // Not sure if required because this will likely be 0 when it powers back up
          helldead();
          leds[0] = CHSV(0, 255, 0);
          FastLED.show();
          esp_deep_sleep_start();
        } else {
          batteryHue = 0;
        }
        break;
      case 1 ... 25:  // Battery is low so red
        batteryHue = 0;
        zeroBattTimes = 0;
        break;
      case 26 ... 50:  // Battery is mid so yellow
        batteryHue = 64;
        zeroBattTimes = 0;
        break;
      case 51 ... 75:  // Battery is high mid so green
        batteryHue = 96;
        zeroBattTimes = 0;
        break;
      case 76 ... 100:  // Battery is decently charged so blue
        batteryHue = 160;
        zeroBattTimes = 0;
        break;
      default:  //Some kind of error maybe so let's just say red
        batteryHue = 0;
        zeroBattTimes = 0;
        break;
    }
  } else {
    batteryReportCounter = batteryReportCounter + 1;
  }
  delay(20);
}

void fakeBattery() {  // Needed a way to test LED conditions
  if (batteryReportCounter == 1500) {
    batteryReportCounter = 50;
  }
  if (batteryReportCounter == 50) {
    Vbattpercent = fakeBatt;
    bleKeyboard.setBatteryLevel(Vbattpercent);
    if (fakeBatt == 0) {
      fakeBatt = 100;
    } else {
      fakeBatt = fakeBatt - 1;
    }
    batteryReportCounter = 0;

    switch (Vbattpercent) {
      case 0:  // Battery is dead - make sure first and then kill the lights and sleep
        helldead();
        leds[0] = CHSV(0, 255, 0);
        FastLED.show();
        esp_deep_sleep_start();
        break;
      case 1 ... 25:  // Battery is low so red
        batteryHue = 0;
        zeroBattTimes = 0;
        break;
      case 26 ... 50:  // Battery is mid so yellow
        batteryHue = 64;
        zeroBattTimes = 0;
        break;
      case 51 ... 75:  // Battery is high mid so green
        batteryHue = 96;
        zeroBattTimes = 0;
        break;
      case 76 ... 100:  // Battery is decently charged so blue
        batteryHue = 160;
        zeroBattTimes = 0;
        break;
      default:  //Some kind of error maybe so let's just say red
        batteryHue = 0;
        zeroBattTimes = 0;
        break;
    }
  } else {
    batteryReportCounter = batteryReportCounter + 1;
  }
  delay(20);
}

#ifdef PS5
void notFound(AsyncWebServerRequest* request) {
  request->send(404, "text/plain", "Not found");
}

void wifiStuff() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", index_html);
  });

  server.on("/get", HTTP_GET, [](AsyncWebServerRequest* request) {
    String inputMessage;
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      passKey = inputMessage.toInt();
      request->send(200, "text/html", "Passkey Sent: " + String(passKey) + " <br>Click <a href=\"/restart\">here</a> to restart the controller");
    } else {
      request->send(200, "text/html", "Passkey Error");
    }
  });

  server.on("/restart", HTTP_GET, [](AsyncWebServerRequest* request) {
    ESP.restart();
  });

  server.onNotFound(notFound);
  server.begin();
}
#endif