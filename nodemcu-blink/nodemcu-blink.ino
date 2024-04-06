// Ran on NodeMCU ESP8266
// Heavily inspired by https://tttapa.github.io/ESP8266/Chap10%20-%20Simple%20Web%20Server.html

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "arduino_secrets.h"

/* Set these to your desired credentials. */
const char *ssid = SECRET_SSID;
const char *password = SECRET_PASS;

// Name Pins
const int greenpin = 12;
const int yellowpin = 13;
const int red1pin = 14;
const int red2pin = 15;

unsigned long startMillis;
unsigned long currentMillis;
bool blinkon = false;

ESP8266WebServer server(80);

void handleRoot() {
        server.send(200, "text/html", "<form action=\"/all_on_full\" method=\"get\" id=\"form1\"></form><button type=\"submit\" form=\"form1\" value=\"On\">On - Full</button><form action=\"/all_on_mid\" method=\"get\" id=\"form2\"></form><button type=\"submit\" form=\"form2\" value=\"On\">On - Mid</button><form action=\"/all_on_blink\" method=\"get\" id=\"form3\"></form><button type=\"submit\" form=\"form3\" value=\"On\">On - Blink</button><form action=\"/all_off\" method=\"get\" id=\"form4\"></form><button type=\"submit\" form=\"form4\" value=\"Off\">Off</button>");
}

void handleNotFound() {
        server.send(404, "text/plain", "404: Not found");
}

void handleReturn() {
        server.sendHeader("Location","/");
        server.send(303);
}

void all_on_full() {
        blinkon = false;
        delay(20);
        handleReturn();
        analogWrite(red1pin, 1024);
        analogWrite(red2pin, 1024);
        analogWrite(yellowpin, 1024);
        analogWrite(greenpin, 1024);
        Serial.println("on - full");
}

void all_on_mid() {
        blinkon = false;
        delay(20);
        handleReturn();
        analogWrite(red1pin, 512);
        analogWrite(red2pin, 512);
        analogWrite(yellowpin, 512);
        analogWrite(greenpin, 512);
        Serial.println("on - mid");
}

void all_off() {
        blinkon = false;
        delay(20);
        handleReturn();
        analogWrite(red1pin, 0);
        analogWrite(red2pin, 0);
        analogWrite(yellowpin, 0);
        analogWrite(greenpin, 0);
        Serial.println("off");
}

void all_on_blink() {
        handleReturn();
        Serial.println("on - blink");
        while (blinkon==true) {
                startMillis = millis();
                currentMillis = startMillis;
                while (currentMillis - startMillis <= 200) {
                        server.handleClient();
                        if (blinkon==true) {
                                analogWrite(red1pin, 1024);
                                analogWrite(red2pin, 1024);
                                analogWrite(yellowpin, 1024);
                                analogWrite(greenpin, 1024);
                                currentMillis = millis();
                        }
                }
                startMillis = millis();
                currentMillis = startMillis;
                while (currentMillis - startMillis <= 200) {
                        server.handleClient();
                        if (blinkon==true) {
                                analogWrite(red1pin, 0);
                                analogWrite(red2pin, 0);
                                analogWrite(yellowpin, 0);
                                analogWrite(greenpin, 0);
                                currentMillis = millis();
                        }
                }
        }
}

void setup() {

        // Set up pins
        pinMode(red1pin, OUTPUT);
        pinMode(red2pin, OUTPUT);
        pinMode(yellowpin, OUTPUT);
        pinMode(greenpin, OUTPUT);

        // Set up and start Serial output and wait for network
        delay(3000);
        Serial.begin(115200);
        Serial.println();
        Serial.print("Connecting to WiFi (" + String(ssid) + ")...");
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
                delay(500);
                Serial.print(".");
        }
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());

        // Start server
        server.on ( "/", handleRoot );
        server.onNotFound(handleNotFound);
        server.begin();
        Serial.println ( "HTTP server started" );

        // Define button actions
        server.on("/all_on_full", []() {
                all_on_full();
                handleRoot();
        });
        server.on("/all_on_mid", []() {
                all_on_mid();
                handleRoot();
        });
        server.on("/all_on_blink", []() {
                blinkon=true;
                all_on_blink();
                handleRoot();
        });
        server.on("/all_off", []() {
                all_off();
                handleRoot();
        });
}

void loop() {
        server.handleClient();
}
