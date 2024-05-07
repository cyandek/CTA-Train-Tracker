#include "WiFi.h"
#include "functions.hpp"
#include "secrets.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <FastLED.h>
#include <HTTPClient.h>
#include <WiFiMulti.h>
#include <iostream>

// Delarations for LED strip
#define NUM_LEDS 66
#define DATA_PIN 4
#define FASTLED_INTERNAL

void WiFi_SmartConfig();
void WiFi_Hardcoded();
void API_Call(JsonDocument &doc);
void Clear_LED();
void Show_LED();
void Clear_Stations();
void NB_Stations_On();
void SB_Stations_On();
void NB_Stations_Blink();
void SB_Stations_Blink();

void setup() { // Setup code to run once
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);

  // WiFi_SmartConfig();
  WiFi_Hardcoded();

  // Add FastLED
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.clearData();
  FastLED.setBrightness(150);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

void loop() { // Main code to run repeatedly

  // Send API request to get train data in JSON format every 15 seconds
  if ((millis() - msLastAPI) > APIinterval) {

    if (debug) {
      Serial.printf("millis - msLastAPI = %d\n", millis() - msLastAPI);
      Serial.println(millis());
    }

    Serial.println("Call API function\n");

    available = false;
    Serial.println("Threads locked\n");

    API_Call(doc); // HTTP rrequest to get json data and store it in json
                   // document called doc that will be passed to threads to
                   // read and control LED based off the train data

    // Reset time
    Serial.println("---- reset msLastAPI----\n");
    msLastAPI = millis();

    available = true;
    Serial.println("Threads unlocked\n");
  }

  if (available == true) {
    Serial.println("Call Show_LED\n");
    Show_LED();
  }

  delay(blinkinterval);
  Serial.printf("Interval over\n");
}
