// This is required because ESP8266 is weird with pins
#define FASTLED_ESP8266_RAW_PIN_ORDER

// Required libraries/files
#include "FastLED.h" // Tested with version 3.3.3
#include "arduino_secrets.h" // Just for Wifi secrets
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Define some constants
#define NUM_LEDS 30
#define DATA_PIN1 D5
#define DATA_PIN2 D6
#define ATTRACT 1
#define CHASE 2
#define FAIL 3
#define BOUNCE 4
#define RCHASE 5
#define BLINK 6
#define BREATHE 7

// Set up our timer and reference to avoid blocking ESP stuff
unsigned long startMillis;
unsigned long currentMillis;

// Set up variables
int EFFECT = ATTRACT;
String effect;
String color1;
String color2;
String subcolor1;
String subcolor2;
String speed;
String width;
String brightness;
int hexcolor1;
int hexcolor2;
int hexsubcolor1;
int hexsubcolor2;
int intspeed;
int intwidth;
int intbrightness;

// Pull our Wifi secrets from arduino_secrets.h
const char *HOSTSSID = SECRET_HOSTSSID;
const char *HOSTPASS = SECRET_HOSTPASS;

// Create or web form - There's got to be a better way
const char *formHtml = "<h1>Plumage</h1><h3>support@fake.email</h3><form action=\"/effectArgs\">\
<label for=\"effect\">Effect:</label><br><select id=\"effect\" name=\"effect\">\
<option value=\"Chase\">Chase</option><option value=\"Random Chase\">Random Chase</option>\
<option value=\"Bounce\">Bounce</option><option value=\"Blink\">Blink</option>\
<option value=\"Breathe\">Breathe</option><option value=\"End\">End Ride (Attract Mode)</option>\
<option value=\"Fail\">Simulate Error for 5 seconds</option></select><br><br>\
<label for=\"color1\">Primary Color 1:</label><br>\
<input type=\"color\" id=\"color1\" name=\"color1\" value=\"#ff0000\"><br><br>\
<label for=\"color2\">Primary Color 2:</label><br>\
<input type=\"color\" id=\"color2\" name=\"color2\" value=\"#00ff00\"><br><br>\
<label for=\"subcolor1\">Secondary Color 1:</label><br>\
<input type=\"color\" id=\"subcolor1\" name=\"subcolor1\" value=\"#0000ff\"><br><br>\
<label for=\"subcolor2\">Secondary Color 2:</label><br>\
<input type=\"color\" id=\"subcolor2\" name=\"subcolor2\" value=\"#ffff00\"><br><br>\
<label for=\"speed\">Effect Speed:</label><br><div class=\"slidecontainer\">\
<input type=\"range\" min=\"-500\" max=\"-30\" value=\"-250\" class=\"slider\" id=\"speed\" name=\"speed\">\
</div><br><label for=\"width\">Width:</label><br><div class=\"slidecontainer\">\
<input type=\"range\" min=\"1\" max=\"5\" value=\"1\" class=\"slider\" id=\"width\" name=\"width\">\
</div><br><label for=\"brightness\">Max Brightness:</label><br><div class=\"slidecontainer\">\
<input type=\"range\" min=\"10\" max=\"255\" value=\"200\" class=\"slider\" id=\"brightness\" name=\"brightness\">\
</div><br><br><input type=\"submit\" value=\"Submit\"></form>";

// Set up LED strip arrays and give our strips names
CRGB leds1[NUM_LEDS];
CRGB leds2[NUM_LEDS];

// Define an insecure web server
ESP8266WebServer server(80);

// Prepare our environment
void setup() {
  // Set up serial output
  Serial.begin(38400);

  // Set up our strips as actual LED strips
  FastLED.addLeds<NEOPIXEL, DATA_PIN1>(leds1, NUM_LEDS);
  FastLED.addLeds<NEOPIXEL, DATA_PIN2>(leds2, NUM_LEDS);
  FastLED.setBrightness(255);

  // Turn off any lingering effects
  Serial.println("");
  Serial.println("Flushing prior effects...");
  for (int i = 0; i < NUM_LEDS; i++) {
    leds1[i] = CRGB::Black;
    leds2[i] = CRGB::Black;
    FastLED.show();
  }

  // Connect to host SSID
  Serial.print("Configuring access point...");
  WiFi.begin(HOSTSSID, HOSTPASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.print("Local IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/effectArgs", handleEffectArgs);
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("Web server is listening...");
}



/////////////////////////////
//  SERVER STUFF GOES HERE //
/////////////////////////////
void handleRoot() {
  server.send(200, "text/html", formHtml);
}

void handleNotFound() {
  Serial.println("Path not found, returning to root.");
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleEffectArgs() {

  effect = server.arg("effect");
  color1 = server.arg("color1");
  color2 = server.arg("color2");
  subcolor1 = server.arg("subcolor1");
  subcolor2 = server.arg("subcolor2");
  speed = server.arg("speed");
  width = server.arg("width");
  brightness = server.arg("brightness");

  color1.remove(0, 1);
  color2.remove(0, 1);
  subcolor1.remove(0, 1);
  subcolor2.remove(0, 1);
  speed.remove(0, 1);

  int hexcolor1 = strtol(color1.c_str(), NULL, 16);
  int hexcolor2 = strtol(color2.c_str(), NULL, 16);
  int hexsubcolor1 = strtol(subcolor1.c_str(), NULL, 16);
  int hexsubcolor2 = strtol(subcolor2.c_str(), NULL, 16);
  int intspeed = strtol(speed.c_str(), NULL, 0);
  int intwidth = strtol(width.c_str(), NULL, 0);
  int intbrightness = strtol(brightness.c_str(), NULL, 0);

  if (server.arg("effect") == "End") {
    server.sendHeader("Location", "/");
    server.send(303);
    Serial.println("Starting Attract effect!");
    EFFECT = ATTRACT;
    attractEffect();
  }
  else if (server.arg("effect") == "Chase") {
    server.sendHeader("Location", "/");
    server.send(303);
    Serial.println("Starting Chase effect!");
    EFFECT = CHASE;
    chaseEffect(hexcolor1, hexcolor2, hexsubcolor1, hexsubcolor2, intspeed, intwidth, intbrightness);
  }
  else if (server.arg("effect") == "Random Chase") {
    server.sendHeader("Location", "/");
    server.send(303);
    Serial.println("Starting Random Chase effect!");
    EFFECT = RCHASE;
    randomChaseEffect(intspeed, intbrightness);
  }
  else if (server.arg("effect") == "Bounce") {
    server.sendHeader("Location", "/");
    server.send(303);
    Serial.println("Starting Bounce effect!");
    EFFECT = BOUNCE;
    bounceEffect(hexcolor1, hexcolor2, hexsubcolor1, hexsubcolor2, intspeed, intwidth, intbrightness);
  }
  else if (server.arg("effect") == "Blink") {
    server.sendHeader("Location", "/");
    server.send(303);
    Serial.println("Starting Blink effect!");
    EFFECT = BLINK;
    blinkEffect(hexcolor1, hexcolor2, intspeed, intbrightness);
  }
  else if (server.arg("effect") == "Breathe") {
    server.sendHeader("Location", "/");
    server.send(303);
    Serial.println("Starting Breathe effect!");
    EFFECT = BREATHE;
    breatheEffect(hexcolor1, hexcolor2, intspeed, intbrightness);
  }
  else {
    server.sendHeader("Location", "/");
    server.send(303);
    Serial.println("Starting Fail effect!");
    EFFECT = FAIL;
    failEffect();
  }
}



/////////////////////////////
//  EFFECT STUFF GOES HERE //
/////////////////////////////
void attractEffect() {
  FastLED.setBrightness(255);
  EFFECT = ATTRACT;
  for (int i = 0; i < NUM_LEDS; i++) {
    startMillis = millis();
    currentMillis = startMillis;
    while (currentMillis - startMillis <= 70) {
      server.handleClient();
      leds1[i] = CRGB::Green;
      leds2[i] = CRGB::Green;
      FastLED.show();
      leds1[i] = CRGB::Blue;
      leds2[i] = CRGB::Blue;
      currentMillis = millis();
    }
  }
  server.handleClient();
  fill_rainbow(leds1, NUM_LEDS, 0, 10);
  fill_rainbow(leds2, NUM_LEDS, 0, 10);
  FastLED.show();
  while (EFFECT == ATTRACT) {
    for (int i = 0; i < NUM_LEDS; i++) {
      startMillis = millis();
      currentMillis = startMillis;
      //      while (currentMillis - startMillis <= 2) {
      server.handleClient();
      if (i != (NUM_LEDS - 1)) {
        leds1[i] = leds1[i + 1];
        leds2[i] = leds2[i + 1];
      }
      else {
        leds1[i] = leds1[0];
        leds2[i] = leds2[0];
      }
      FastLED.show();
      //    currentMillis = millis();
      //    }
    }
  }
}

void chaseEffect(int color1, int color2, int subcolor1, int subcolor2, int speed, int width, int brightness) {
  FastLED.setBrightness(brightness);
  while (EFFECT == CHASE) {
    for (int i = 0; i <= (NUM_LEDS - width); i++) {
      startMillis = millis();
      currentMillis = startMillis;
      while (currentMillis - startMillis <= speed) {
        switch (width) {
          case 1:
            server.handleClient();
            leds1[i] = color1;
            leds2[i] = color2;
            FastLED.show();
            leds1[i] = subcolor1;
            leds2[i] = subcolor2;
            currentMillis = millis();
            break;
          case 2:
            server.handleClient();
            leds1[i] = color1;
            leds2[i] = color2;
            leds1[(i + 1)] = color1;
            leds2[(i + 1)] = color2;
            FastLED.show();
            leds1[i] = subcolor1;
            leds2[i] = subcolor2;
            leds1[(i + 1)] = subcolor1;
            leds2[(i + 1)] = subcolor2;
            currentMillis = millis();
            break;
          case 3:
            server.handleClient();
            leds1[i] = color1;
            leds2[i] = color2;
            leds1[(i + 1)] = color1;
            leds2[(i + 1)] = color2;
            leds1[(i + 2)] = color1;
            leds2[(i + 2)] = color2;
            FastLED.show();
            leds1[i] = subcolor1;
            leds2[i] = subcolor2;
            leds1[(i + 1)] = subcolor1;
            leds2[(i + 1)] = subcolor2;
            leds1[(i + 2)] = subcolor1;
            leds2[(i + 2)] = subcolor2;
            currentMillis = millis();
            break;
          case 4:
            server.handleClient();
            leds1[i] = color1;
            leds2[i] = color2;
            leds1[(i + 1)] = color1;
            leds2[(i + 1)] = color2;
            leds1[(i + 2)] = color1;
            leds2[(i + 2)] = color2;
            leds1[(i + 3)] = color1;
            leds2[(i + 3)] = color2;
            FastLED.show();
            leds1[i] = subcolor1;
            leds2[i] = subcolor2;
            leds1[(i + 1)] = subcolor1;
            leds2[(i + 1)] = subcolor2;
            leds1[(i + 2)] = subcolor1;
            leds2[(i + 2)] = subcolor2;
            leds1[(i + 3)] = subcolor1;
            leds2[(i + 3)] = subcolor2;
            currentMillis = millis();
            break;
          case 5:
            server.handleClient();
            leds1[i] = color1;
            leds2[i] = color2;
            leds1[(i + 1)] = color1;
            leds2[(i + 1)] = color2;
            leds1[(i + 2)] = color1;
            leds2[(i + 2)] = color2;
            leds1[(i + 3)] = color1;
            leds2[(i + 3)] = color2;
            leds1[(i + 4)] = color1;
            leds2[(i + 4)] = color2;
            FastLED.show();
            leds1[i] = subcolor1;
            leds2[i] = subcolor2;
            leds1[(i + 1)] = subcolor1;
            leds2[(i + 1)] = subcolor2;
            leds1[(i + 2)] = subcolor1;
            leds2[(i + 2)] = subcolor2;
            leds1[(i + 3)] = subcolor1;
            leds2[(i + 3)] = subcolor2;
            leds1[(i + 4)] = subcolor1;
            leds2[(i + 4)] = subcolor2;
            currentMillis = millis();
            break;
          default:
            break;
        }
        server.handleClient();
        leds1[i] = color1;
        leds2[i] = color2;
        FastLED.show();
        leds1[i] = subcolor1;
        leds2[i] = subcolor2;
        currentMillis = millis();
      }
    }
  }
}

void randomChaseEffect(int speed, int brightness) {
  FastLED.setBrightness(brightness);
  while (EFFECT == RCHASE) {
    int rcolor1 = random(0, 16777215);
    int rcolor2 = random(0, 16777215);
    int rsubcolor1 = random(0, 16777215);
    int rsubcolor2 = random(0, 16777215);
    for (int i = 0; i < NUM_LEDS; i++) {
      startMillis = millis();
      currentMillis = startMillis;
      while (currentMillis - startMillis <= speed) {
        server.handleClient();
        leds1[i] = rcolor1;
        leds2[i] = rcolor2;
        FastLED.show();
        leds1[i] = rsubcolor1;
        leds2[i] = rsubcolor2;
        currentMillis = millis();
      }
    }
  }
}

void bounceEffect(int color1, int color2, int subcolor1, int subcolor2, int speed, int width, int brightness) {
  FastLED.setBrightness(brightness);
  while (EFFECT == BOUNCE) {
    for (int i = 0; i <= (NUM_LEDS - width); i++) {
      startMillis = millis();
      currentMillis = startMillis;
      while (currentMillis - startMillis <= speed) {
        switch (width) {
          case 1:
            server.handleClient();
            leds1[i] = color1;
            leds2[i] = color2;
            FastLED.show();
            leds1[i] = subcolor1;
            leds2[i] = subcolor2;
            currentMillis = millis();
            break;
          case 2:
            server.handleClient();
            leds1[i] = color1;
            leds2[i] = color2;
            leds1[(i + 1)] = color1;
            leds2[(i + 1)] = color2;
            FastLED.show();
            leds1[i] = subcolor1;
            leds2[i] = subcolor2;
            leds1[(i + 1)] = subcolor1;
            leds2[(i + 1)] = subcolor2;
            currentMillis = millis();
            break;
          case 3:
            server.handleClient();
            leds1[i] = color1;
            leds2[i] = color2;
            leds1[(i + 1)] = color1;
            leds2[(i + 1)] = color2;
            leds1[(i + 2)] = color1;
            leds2[(i + 2)] = color2;
            FastLED.show();
            leds1[i] = subcolor1;
            leds2[i] = subcolor2;
            leds1[(i + 1)] = subcolor1;
            leds2[(i + 1)] = subcolor2;
            leds1[(i + 2)] = subcolor1;
            leds2[(i + 2)] = subcolor2;
            currentMillis = millis();
            break;
          case 4:
            server.handleClient();
            leds1[i] = color1;
            leds2[i] = color2;
            leds1[(i + 1)] = color1;
            leds2[(i + 1)] = color2;
            leds1[(i + 2)] = color1;
            leds2[(i + 2)] = color2;
            leds1[(i + 3)] = color1;
            leds2[(i + 3)] = color2;
            FastLED.show();
            leds1[i] = subcolor1;
            leds2[i] = subcolor2;
            leds1[(i + 1)] = subcolor1;
            leds2[(i + 1)] = subcolor2;
            leds1[(i + 2)] = subcolor1;
            leds2[(i + 2)] = subcolor2;
            leds1[(i + 3)] = subcolor1;
            leds2[(i + 3)] = subcolor2;
            currentMillis = millis();
            break;
          case 5:
            server.handleClient();
            leds1[i] = color1;
            leds2[i] = color2;
            leds1[(i + 1)] = color1;
            leds2[(i + 1)] = color2;
            leds1[(i + 2)] = color1;
            leds2[(i + 2)] = color2;
            leds1[(i + 3)] = color1;
            leds2[(i + 3)] = color2;
            leds1[(i + 4)] = color1;
            leds2[(i + 4)] = color2;
            FastLED.show();
            leds1[i] = subcolor1;
            leds2[i] = subcolor2;
            leds1[(i + 1)] = subcolor1;
            leds2[(i + 1)] = subcolor2;
            leds1[(i + 2)] = subcolor1;
            leds2[(i + 2)] = subcolor2;
            leds1[(i + 3)] = subcolor1;
            leds2[(i + 3)] = subcolor2;
            leds1[(i + 4)] = subcolor1;
            leds2[(i + 4)] = subcolor2;
            currentMillis = millis();
            break;
          default:
            break;
        }
        server.handleClient();
        leds1[i] = color1;
        leds2[i] = color2;
        FastLED.show();
        leds1[i] = subcolor1;
        leds2[i] = subcolor2;
        currentMillis = millis();
      }
    }
    for (int i = (NUM_LEDS - 1); i >= (width - 1); i--) {
      startMillis = millis();
      currentMillis = startMillis;
      while (currentMillis - startMillis <= speed) {
        switch (width) {
          case 1:
            server.handleClient();
            leds1[i] = color1;
            leds2[i] = color2;
            FastLED.show();
            leds1[i] = subcolor1;
            leds2[i] = subcolor2;
            currentMillis = millis();
            break;
          case 2:
            server.handleClient();
            leds1[i] = color1;
            leds2[i] = color2;
            leds1[(i - 1)] = color1;
            leds2[(i - 1)] = color2;
            FastLED.show();
            leds1[i] = subcolor1;
            leds2[i] = subcolor2;
            leds1[(i - 1)] = subcolor1;
            leds2[(i - 1)] = subcolor2;
            currentMillis = millis();
            break;
          case 3:
            server.handleClient();
            leds1[i] = color1;
            leds2[i] = color2;
            leds1[(i - 1)] = color1;
            leds2[(i - 1)] = color2;
            leds1[(i - 2)] = color1;
            leds2[(i - 2)] = color2;
            FastLED.show();
            leds1[i] = subcolor1;
            leds2[i] = subcolor2;
            leds1[(i - 1)] = subcolor1;
            leds2[(i - 1)] = subcolor2;
            leds1[(i - 2)] = subcolor1;
            leds2[(i - 2)] = subcolor2;
            currentMillis = millis();
            break;
          case 4:
            server.handleClient();
            leds1[i] = color1;
            leds2[i] = color2;
            leds1[(i - 1)] = color1;
            leds2[(i - 1)] = color2;
            leds1[(i - 2)] = color1;
            leds2[(i - 2)] = color2;
            leds1[(i - 3)] = color1;
            leds2[(i - 3)] = color2;
            FastLED.show();
            leds1[i] = subcolor1;
            leds2[i] = subcolor2;
            leds1[(i - 1)] = subcolor1;
            leds2[(i - 1)] = subcolor2;
            leds1[(i - 2)] = subcolor1;
            leds2[(i - 2)] = subcolor2;
            leds1[(i - 3)] = subcolor1;
            leds2[(i - 3)] = subcolor2;
            currentMillis = millis();
            break;
          case 5:
            server.handleClient();
            leds1[i] = color1;
            leds2[i] = color2;
            leds1[(i - 1)] = color1;
            leds2[(i - 1)] = color2;
            leds1[(i - 2)] = color1;
            leds2[(i - 2)] = color2;
            leds1[(i - 3)] = color1;
            leds2[(i - 3)] = color2;
            leds1[(i - 4)] = color1;
            leds2[(i - 4)] = color2;
            FastLED.show();
            leds1[i] = subcolor1;
            leds2[i] = subcolor2;
            leds1[(i - 1)] = subcolor1;
            leds2[(i - 1)] = subcolor2;
            leds1[(i - 2)] = subcolor1;
            leds2[(i - 2)] = subcolor2;
            leds1[(i - 3)] = subcolor1;
            leds2[(i - 3)] = subcolor2;
            leds1[(i - 4)] = subcolor1;
            leds2[(i - 4)] = subcolor2;
            currentMillis = millis();
            break;
          default:
            break;
        }
        server.handleClient();
        leds1[i] = color1;
        leds2[i] = color2;
        FastLED.show();
        leds1[i] = subcolor1;
        leds2[i] = subcolor2;
        currentMillis = millis();
      }
    }
  }
}

void blinkEffect(int color1, int color2, int speed, int brightness) {
  FastLED.setBrightness(brightness);
  while (EFFECT == BLINK) {
    startMillis = millis();
    currentMillis = startMillis;
    while (currentMillis - startMillis <= speed) {
      server.handleClient();
      fill_solid(leds1, NUM_LEDS, color1);
      fill_solid(leds2, NUM_LEDS, color2);
      FastLED.show();
      currentMillis = millis();
    }
    startMillis = millis();
    currentMillis = startMillis;
    while (currentMillis - startMillis <= speed) {
      server.handleClient();
      fill_solid(leds1, NUM_LEDS, color2);
      fill_solid(leds2, NUM_LEDS, color1);
      FastLED.show();
      currentMillis = millis();
    }
  }
}

void breatheEffect(int color1, int color2, int speed, int brightness) {
  while (EFFECT == BREATHE) {
    fill_solid(leds1, NUM_LEDS, color1);
    fill_solid(leds2, NUM_LEDS, color2);
    FastLED.show();
    for (int i = brightness; i > 0; i = (i - 3)) {
      startMillis = millis();
      currentMillis = startMillis;
      while (currentMillis - startMillis <= speed) {
        server.handleClient();
        FastLED.setBrightness(i);
        FastLED.show();
        currentMillis = millis();
      }
    }
    for (int i = 0; i < brightness; i = (i + 3)) {
      startMillis = millis();
      currentMillis = startMillis;
      while (currentMillis - startMillis <= speed) {
        server.handleClient();
        FastLED.setBrightness(i);
        FastLED.show();
        currentMillis = millis();
      }
    }
  }
}

void failEffect() {
  FastLED.setBrightness(255);
  for (int t = 0; t < 10; t++) {
    startMillis = millis();
    currentMillis = startMillis;
    while (currentMillis - startMillis <= 250) {
      server.handleClient();
      fill_solid(leds1, NUM_LEDS, CRGB::Red);
      fill_solid(leds2, NUM_LEDS, CRGB::Black);
      FastLED.show();
      currentMillis = millis();
    }
    startMillis = millis();
    currentMillis = startMillis;
    while (currentMillis - startMillis <= 250) {
      server.handleClient();
      fill_solid(leds1, NUM_LEDS, CRGB::Black);
      fill_solid(leds2, NUM_LEDS, CRGB::Red);
      FastLED.show();
      currentMillis = millis();
    }
  }
  EFFECT = ATTRACT;
  attractEffect();
}

// Prepare to be amazed
void loop() {
  attractEffect();
}
