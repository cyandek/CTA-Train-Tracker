#include <Arduino.h>
#include "functions.hpp"
#include "secrets.h"
#include <ArduinoJson.h>
#include <FastLED.h>
#include <HTTPClient.h>
#include <WiFiMulti.h>
#include <iostream>

// Delarations for LED strip
#define NUM_LEDS 33
#define DATA_PIN 4
#define FASTLED_INTERNAL

void WiFi_SmartConfig();
void WiFi_Hardcoded();
void API_Call(JsonDocument &doc);
void Clear_LED();
void Show_LED();


void setup() { 
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(DATA_PIN, OUTPUT);

    WiFi_SmartConfig();        //Configure WiFi with Espressif Esptouch app
    // WiFi_Hardcoded();       //Configure WiFi with hardcoded SSID and Password in secrets.h

    // Add FastLED
    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
    FastLED.clear();
    FastLED.clearData();
    FastLED.setBrightness(150);
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    //Serial.println("All LEDs should be off now (setup)");

}

void loop() { 
// Send API request to get train data in JSON format every 15 seconds
    if ((millis() - msLastAPI) > APIinterval) {
      //Serial.printf("millis - msLastAPI = %d\n", millis() - msLastAPI);
      // Serial.println(millis());

      Serial.println("Call API function\n");

      available = false;
      Serial.println("Locked\n");

      API_Call(doc); // HTTP request to get json data and store it in json
                     // document called doc that will be passed to functions to
                     // read and control LED based off the train data

      // Reset time
      //Serial.println("---- reset msLastAPI----\n");
      msLastAPI = millis();

      available = true;
      Serial.println("Unlocked\n");
    }
    delay(blinkinterval);
    //Serial.printf("Blink interval over\n");

    if (available == true) {
      Serial.println("Call Show_LED\n");
      Show_LED();
    }
}
