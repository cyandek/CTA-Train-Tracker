#include "WiFi.h"
#include "declarations.hpp"
#include "secrets.h"
#include <ArduinoJson.h>
#include <FastLED.h>
#include <HTTPClient.h>
#include <WiFiMulti.h>
#include <iostream>
#include <string>
#include <time.h>

// Function to configure the WiFI via the Espressif Esptouch app
void WiFi_SmartConfig() {

  // Init WiFi as Station, start SmartConfig
  WiFi.mode(WIFI_AP_STA);
  WiFi.beginSmartConfig();

  // Wait for SmartConfig packet from mobile
  Serial.println("Waiting for SmartConfig.");
  while (!WiFi.smartConfigDone()) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("SmartConfig received.");

  // Wait for WiFi to connect to AP
  Serial.println("Waiting for WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi Connected.");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// Function to configure the WiFi via hardcoded credentials in secrets.h
void WiFi_Hardcoded() {

  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  while (wifiMulti.run() != WL_CONNECTED) {
    delay(100);
    WiFi.mode(WIFI_STA); // To connect to iPhone hotspot
    WiFi.disconnect();
    WiFi.reconnect();
  }

  // Print connected to wifi
  digitalWrite(LED_BUILTIN, WiFi.status() == WL_CONNECTED);
  Serial.println("Wifi Connected");
  delay(100);
}

// Clear LED strip
void Clear_LED() {
  FastLED.clear();
  FastLED.clearData();
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  Serial.println("All LEDs should be off now");
}

// Set status of all stations to off
void Clear_Stations() {
  Serial.println("Clear station status");

  // Clear status for SB stations
  for (int y = 0; y < SBarr_size;
       y++) // Loop through stopIDs (stations) and set status to off
  {
    SB_station[1][y] = off;
    SB_pulse[y] = off;
  }

  // Clear status for NB stations
  for (int y = 0; y < NBarr_size;
       y++) // Loop through stopIDs (stations) and set status to off
  {
    NB_station[1][y] = off;
    NB_pulse[y] = off;
  }
}

// Loop through the 2D array and turn on the Southbound LEDs that have a status
// of 'on'
void SB_Stations_On() {
  Serial.printf("SB_Stations_On start\n");

  // Southbound
  for (int y = 0; y < SBarr_size; y++) {

    if (SB_station[1][y] == on) {
      Serial.printf("SB LED on - %s", Station_names[y]);
      Serial.printf(", index #: %d\n", y);
      leds[y] = CRGB::Red; // Set LED to Red
      FastLED.show();      // Turn on Station LEDs
    }
  }
}

// Loop through the 2D array and turn on the Northbound LEDs that have a status
// of 'on'
void NB_Stations_On() {
  Serial.printf("NB_Stations_On start\n");
  // Northbound
  for (int y = 0; y < NBarr_size; y++) {

    if (NB_station[1][y] == on) {
      Serial.printf("NB LED on - %s", Station_names[y]);
      Serial.printf(", index #: %d\n", y + SBarr_size);
      leds[y + SBarr_size] = CRGB::Red; // Set LED to Red
      FastLED.show();                   // Turn on Station LEDs
    }
  }
}

// Loop through the 2D array and toggle the Southbound LEDs that have a status
// of 'blink'
void SB_Stations_Blink() {
  Serial.printf("SB_Stations_Blink\n");

  // Southbound
  for (int y = 0; y < SBarr_size; y++) {
    if (SB_station[1][y] == blink) {
      if (SB_pulse[y] == off) { // If LED off turn it on
        Serial.printf("SB LED Blink - %s", Station_names[y]);
        Serial.printf(", index #: %d\n", y);
        leds[y] = CRGB::Red; // Set LED to Red
        // leds[y].subtractFromRGB(25); // Blink at a dim red?
        FastLED.show();   // Turn on Station LEDs
        SB_pulse[y] = on; // Set status to on
      } else {            // If LED on turn it off
        Serial.printf("SB LED Blink - %s", Station_names[y]);
        Serial.printf(", index #: %d\n", y);
        leds[y] = CRGB::Black; // Set LED to Black
        // leds[y].fadeToBlackBy(fadeAmount); // Dim LED to near black
        FastLED.show();    // Turn on Station LEDs
        SB_pulse[y] = off; // Set status to off
      }
    }
  }
}

// Loop through the 2D array and toggle the Northbound LEDs that have a status
// of 'blink'
void NB_Stations_Blink() {
  Serial.printf("NB_Stations_Blink\n");
  // Northbound

  for (int y = 0; y < NBarr_size; y++) {
    if (NB_station[1][y] == blink) {
      if (NB_pulse[y] == off) { // If LED off turn it on
         Serial.printf("NB LED Blink - %s", Station_names[y]);
         Serial.printf(", index #: %d\n", y + SBarr_size);
        leds[y + SBarr_size] = CRGB::Red; // Set LED to Red
        // leds[y + SBarr_size].subtractFromRGB(25); // Blink at a dim red?
        FastLED.show();   // Turn on Station LEDs
        NB_pulse[y] = on; // Set status to on
      } else {            // If LED on turn it off
         Serial.printf("NB LED Blink - %s", Station_names[y]);
         Serial.printf(", index #: %d\n", y+ SBarr_size);
        leds[y + SBarr_size] = CRGB::Black; // Set LED to Black
        // leds[y + SBarr_size].fadeToBlackBy(fadeAmount); // Dim LED to near
        // black
        FastLED.show();    // Turn on Station LEDs
        NB_pulse[y] = off; // Set status to off
      }
    }
  }
}

// Send API request
void API_Call(JsonDocument &doc) {

  Serial.println("Start API_Call\n");

  doc.clear(); // Clear json document

  if (WiFi.status() == WL_CONNECTED) {            // If Wifi connected send http request
    if (debug){
    Serial.println("Wifi is connected");
    Serial.println(WiFi.localIP());
    }
    http.begin(client, API_URL);
    http.GET();
  } else {                                        // If Wifi not connected try to connect with hardcoded credentials
    if (debug) { 
      Serial.println("Wifi is not connected");
      Serial.println("WiFi_Hardcoded starting");
    }
    WiFi_Hardcoded();
    if (WiFi.status() == WL_CONNECTED) {          // If Wifi connected send http request
      if (debug) {
        Serial.println("Wifi is connected");
        Serial.println(WiFi.localIP());
      }
      http.begin(client, API_URL);
      http.GET();
    } else {                                       // If Wifi still not connected try to connect via Espressif EspTouch app
      if (debug) {
        Serial.println("Wifi is not connected");
        Serial.println("WiFi_SmartConfig starting");
      }
      WiFi_SmartConfig();
    }
  }

      String input = http.getString();

      JsonDocument filter;

      printf("Filtering JSON\n");

      JsonObject filter_train =
          filter["ctatt"]["route"][0]["train"].add<JsonObject>();
      filter_train["trDr"] = true;
      filter_train["nextStpId"] = true;
      filter_train["nextStaNm"] = true;
      filter_train["isApp"] = true;
      filter_train["arrT"] = true;
      filter_train["prdt"] = true;

      DeserializationError error =
          deserializeJson(doc, input, DeserializationOption::Filter(filter));

      // Print the result
      serializeJsonPretty(doc, Serial);
      printf("\n");

      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
      }

      Clear_LED();
      Clear_Stations();

      Serial.printf("Start LED on \n");
      for (int i = 0; i + 1 < 20;
           i++) // Cycle through up to 20 trains from the json response
      {
        if (debug) {
          Serial.printf("\n");
          Serial.printf("Train:%d ", i); // Print train number
        }
        direction = doc["ctatt"]["route"][0]["train"][i]["trDr"];
        isApp = doc["ctatt"]["route"][0]["train"][i]["isApp"];
        nextStpId = doc["ctatt"]["route"][0]["train"][i]["nextStpId"];
        nextStationName = doc["ctatt"]["route"][0]["train"][i]["nextStaNm"];
        arrT = doc["ctatt"]["route"][0]["train"][i]["arrT"];
        prdt = doc["ctatt"]["route"][0]["train"][i]["prdt"];

        // Set station status for SB Trains at Station
        if (direction == SBRed &&
            isApp == on) // Confirm train is Southbound & at station
        {
          for (int y = 0; y + 1 < SBarr_size;
               y++) // Loop through stopIDs (stations)
          {
            if (debug) {
              Serial.printf("y:%d ", y);
            }
            if (nextStpId ==
                SB_station[0][y]) // If current value is equal to
                                  // Stop ID, set the station status to on
            {

              if (debug) {
                Serial.printf("\n");
                Serial.printf("SB LED On - %s",
                              nextStationName);      // print station to turn on
                Serial.printf(", index #: %d\n", y); // print led index
              }

              SB_station[1][y] = on; // set station status to on
            }
          }
        }

        // Set station status for NB Trains at Station
        if (direction == NBRed &&
            isApp == on) // Confirm train is Northbound & at station
        {
          for (int y = 0; y + 1 < NBarr_size;
               y++) // Loop through stopIDs (stations)
          {
            if (debug) {
              Serial.printf("y:%d ", y);
            }
            if (nextStpId ==
                NB_station[0][y]) // If current value is equal to
                                  // Stop ID, set the station status to on
            {

              if (debug) {
                Serial.printf("\n");
                Serial.printf("NB LED On - %s",
                              nextStationName); // print station to turn on
                Serial.printf(", index #: %d\n",
                              y + SBarr_size); // print led index
              }

              NB_station[1][y] = on; // set station status to on
            }
          }
        }
      }
      Serial.printf("End LED on \n");

      Serial.printf("Start LED blink \n");
      for (int i = 0; i + 1 < 20;
           i++) // Cycle through up to 20 trains from the json response
      {
        if (debug) {
          Serial.printf("\n");
          Serial.printf("Train:%d ", i);
        }

        direction = doc["ctatt"]["route"][0]["train"][i]["trDr"];
        isApp = doc["ctatt"]["route"][0]["train"][i]["isApp"];
        nextStpId = doc["ctatt"]["route"][0]["train"][i]["nextStpId"];
        nextStationName = doc["ctatt"]["route"][0]["train"][i]["nextStaNm"];
        arrT = doc["ctatt"]["route"][0]["train"][i]["arrT"];
        prdt = doc["ctatt"]["route"][0]["train"][i]["prdt"];

        if (direction == SBRed &&
            isApp ==
                0) // Confirm train is southbound and not currently at a station
        {
          if (debug) {
            printf("SB and not at station - Train:%d \n", i);
          }

          //  Format Prediction Time
          struct tm tm = {0};
          strptime(prdt, "%Y-%m-%dT%H:%M:%S",
                   &tm); // Convert to tm struct
          char pred[100];
          strftime(pred, sizeof(pred), "%d %b %Y %H:%M",
                   &tm); // Convert format

          if (debug) {
            Serial.printf("SB Blink Station: %s", nextStationName);
            Serial.printf(" Predition Time: %s\n", pred);
          }
          time_t predt = mktime(&tm); // Convert to time_t

          // Format Arrival Time
          char arrive[100]; // Format Arrival Time
          strptime(arrT, "%Y-%m-%dT%H:%M:%S",
                   &tm); // Convert to tm struct
          strftime(arrive, sizeof(arrive), "%d %b %Y %H:%M",
                   &tm); // Convert format
          if (debug) {
            Serial.printf("Blink Station: %s", nextStationName);
          }
          time_t arrivet = mktime(&tm);

          if (difftime(predt, arrivet) <= 180) // Confirm train is arriving at
                                               // station in less than 3 mins
          {
            if (debug) {
              Serial.printf("SB and arriving <3 min - Train: %d\n", i);
            }

            // Find index of Stop ID
            for (int y = 0; y + 1 < SBarr_size;
                 y++) // Loop through stopIDs (stations)
            {
              if (debug) {
                Serial.printf("y:%d ", y);
              }
              if (nextStpId == SB_station[0][y]) // If current value is
                                                 // equal to Stop ID set station
                                                 // status to blink
              {
                if (debug) {
                  Serial.printf("\n");
                  Serial.printf("SB LED Blink - %s", nextStationName);
                  Serial.printf(", index #: %d\n", y);
                }

                SB_station[1][y] = blink; // set status to blink
              }
            }
          } else {
            for (int y = 0; y + 1 < SBarr_size;
                 y++) // Loop through stopIDs (stations)
            {
              if (debug) {
                Serial.printf("y:%d ", y);
              }
              if (nextStpId == SB_station[0][y]) // If current value is
                                                 // equal to Stop ID set station
                                                 // status to off
              {
                if (debug) {
                  Serial.printf("\n");
                  Serial.printf("SB LED Off - %s", nextStationName);
                  Serial.printf(", index #: %d\n", y);
                }

                SB_station[1][y] = off; // set status to off
              }
            }
          }
        }

        if (direction == NBRed && isApp == 0) // Confirm train is northbound and
                                              // not currently at a station
        {
          if (debug) {
            printf("NB and not at station - Train:%d \n", i);
          }

          //  Format Prediction Time
          struct tm tm = {0};
          strptime(prdt, "%Y-%m-%dT%H:%M:%S",
                   &tm); // Convert to tm struct
          char pred[100];
          strftime(pred, sizeof(pred), "%d %b %Y %H:%M",
                   &tm); // Convert format

          if (debug) {
            Serial.printf("NB Blink Station: %s", nextStationName);
            Serial.printf(" Predition Time: %s\n", pred);
          }
          time_t predt = mktime(&tm); // Convert to time_t

          // Format Arrival Time
          char arrive[100]; // Format Arrival Time
          strptime(arrT, "%Y-%m-%dT%H:%M:%S",
                   &tm); // Convert to tm struct
          strftime(arrive, sizeof(arrive), "%d %b %Y %H:%M",
                   &tm); // Convert format
          if (debug) {
            Serial.printf("NB Blink Station: %s", nextStationName);
          }
          time_t arrivet = mktime(&tm);

          if (difftime(predt, arrivet) <= 180) // Confirm train is arriving at
                                               // station in less than 3 mins
          {
            if (debug) {
              Serial.printf("NB and arriving <3 min - Train: %d\n", i);
            }

            // Find index of Stop ID
            for (int y = 0; y + 1 < NBarr_size;
                 y++) // Loop through stopIDs (stations)
            {
              if (debug) {
                Serial.printf("y:%d ", y);
              }
              if (nextStpId ==
                  NB_station[0][y]) // If current value is
                                    // equal to Stop ID set station to blink
              {
                if (debug) {
                  Serial.printf("\n");
                  Serial.printf("NB LED Blink - %s", nextStationName);
                  Serial.printf(", index #: %d\n", y + SBarr_size);
                }

                NB_station[1][y] = blink; // set status to blink
              }
            }
          } else {
            for (int y = 0; y + 1 < NBarr_size;
                 y++) // Loop through stopIDs (stations)
            {
              if (debug) {
                Serial.printf("y:%d ", y);
              }
              if (nextStpId == NB_station[0][y]) // If current value is
                                                 // equal to Stop ID
              {
                if (debug) {
                  Serial.printf("\n");
                  Serial.printf("NB LED Off - %s", nextStationName);
                  Serial.printf(", index #: %d\n", y + SBarr_size);
                }

                NB_station[1][y] = off; // set status to off
              }
            }
          }
        }
      }
      Serial.printf("End LED blink \n");
      http.end();
      Serial.println("End API_Call\n");
    }

void Show_LED() {
  Serial.println("Start Show_LED\n");
  using namespace std;

  if (debug) {
    for (int i = 1; i < 2; ++i) {

      // Print SB_station array
      for (int j = 0; j < 33; ++j) {
        cout << "SB_station[" << i << "][" << j << "] = " << SB_station[i][j]
             << endl;
      }

      // Print NB_station array
      for (int j = 0; j < 33; ++j) {
        cout << "NB_station[" << i << "][" << j << "] = " << NB_station[i][j]
             << endl;
      }
    }
  }

  NB_Stations_On();
  NB_Stations_Blink();
  SB_Stations_On();
  SB_Stations_Blink();

  Serial.println("End Show_LED\n");
}
