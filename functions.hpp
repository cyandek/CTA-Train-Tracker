#include "WiFi.h"
#include "declarations.hpp"
#include "secrets.h"
#include <ArduinoJson.h>
#include <FastLED.h>
#include <HTTPClient.h>
#include <iostream>
#include <string>
#include <time.h>

void WiFi_SmartConfig(){

  // Init WiFi as Station, start SmartConfig 
  WiFi.mode(WIFI_AP_STA);
  WiFi.beginSmartConfig();

  // Wait for SmartConfig packet from mobile
  Serial.println("Waiting for SmartConfig.");
  while (!WiFi.smartConfigDone()) {
    delay(500);
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

void WiFi_Hardcoded(){

wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
wifiMulti.addAP(WIFI_SSID_PHONE, WIFI_PASSWORD_PHONE);
wifiMulti.addAP(WIFI_SSID_LP, WIFI_PASSWORD_LP);

while (wifiMulti.run() != WL_CONNECTED)
{
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
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  Serial.println("All LEDs should be off now");
}

// Set status of all stations to off
void Clear_Stations() {
  Serial.printf("Clear station status");
  for (int y = 0; y + 1 < SBarr_size; y++) // Loop through stopIDs (stations)
  {
    SB_station[1][y] = off;
  }
}

// Set status of all stations to on if Southbound Train is approaching the
// station
void Stations_On() {
  Serial.printf("Stations_On start\n");
  for (int x = 1; x < 2; x++) {
    for (int y = 0; y < SBarr_size; y++) {

      if (SB_station[x][y] == on) {
        Serial.printf("LED on, index #: %d\n", y);
        leds[y] = CRGB::Red; // Set LED to Red
        FastLED.show(); // Turn on Station LEDs
      }
    }
  }
}

// Set status of all stations to blink if Southbound Train is arriving in < 3
// min
void Stations_Blink() {
  Serial.printf("Show_LED blink\n");
  for (int x = 1; x < 2; x++) {
    for (int y = 0; y < SBarr_size; y++) {

      if (SB_station[x][y] == blink) {
        if (pulseType[y] == off) { // If LED off turn it on
          Serial.printf("LED Blink, index #: %d\n", y);
          leds[y] = CRGB::Red; // Set LED to Red
          //leds[y].subtractFromRGB(25); // Blink at a dim red?
          FastLED.show();      // Turn on Station LEDs
          pulseType[y] = on;   // Set switch to on
        } else {
          Serial.printf("LED Blink, index #: %d\n", y);
          leds[y] = CRGB::Black; // Set LED to Black
          //leds[y].fadeToBlackBy(fadeAmount); // Dim LED to near black
          FastLED.show();     // Turn on Station LEDs
          pulseType[y] = off; // Set switch to off
        }
      }
    }
  }
}

// Send API request
void API_Call(JsonDocument &doc) {

  Serial.println("Start API_Call\n");

  http.useHTTP10(true);
  http.begin(client, API_URL);
  http.GET();

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
  for (int i = 0; i + 1 < 17; i++) // Cycle through the 17 trains
  {
    // Serial.printf("i: %d", i);
    direction = doc["ctatt"]["route"][0]["train"][i]["trDr"];
    isApp = doc["ctatt"]["route"][0]["train"][i]["isApp"];
    nextStpId = doc["ctatt"]["route"][0]["train"][i]["nextStpId"];
    nextStationName = doc["ctatt"]["route"][0]["train"][i]["nextStaNm"];
    arrT = doc["ctatt"]["route"][0]["train"][i]["arrT"];
    prdt = doc["ctatt"]["route"][0]["train"][i]["prdt"];

    if (direction == SBRed &&
        isApp == on) // Confirm train is Southbound & at station
    {
      for (int y = 0; y + 1 < SBarr_size;
           y++) // Loop through stopIDs (stations)
      {
        // Serial.printf("y: %d", y);
        if (nextStpId == SB_station[0][y]) // If current value is equal to
                                           // Stop ID, set the index value
        {
          /*           Serial.printf("\n");
                    Serial.printf("LED On - %s", nextStationName);
                    Serial.printf(", index #: %d\n", y); */
          SB_station[1][y] = on;
        }
      }
    }
  }
  Serial.printf("End LED on \n");

  Serial.printf("Start LED blink \n");
  for (int i = 0; i + 1 < 17; i++) // Cycle through the 17 trains
  {
    /*     Serial.printf("i: %d", i); */
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
      // printf("SB and not at station - Train: %d\n", i);
      //  Format Prediction Time
      struct tm tm = {0};
      strptime(prdt, "%Y-%m-%dT%H:%M:%S",
               &tm); // Convert to tm struct
      char pred[100];
      strftime(pred, sizeof(pred), "%d %b %Y %H:%M",
               &tm); // Convert format

      /*       Serial.printf("Blink Station: %s", nextStationName);
            Serial.printf(" Predition Time: %s\n", pred); */
      time_t predt = mktime(&tm); // Convert to time_t

      // Format Arrival Time
      char arrive[100]; // Format Arrival Time
      strptime(arrT, "%Y-%m-%dT%H:%M:%S",
               &tm); // Convert to tm struct
      strftime(arrive, sizeof(arrive), "%d %b %Y %H:%M",
               &tm); // Convert format
      /* Serial.printf("Blink Station: %s", nextStationName); */
      time_t arrivet = mktime(&tm);

      if (difftime(predt, arrivet) <= 180) // Confirm train is arriving at
                                           // station in less than 3 mins
      {
        // Serial.printf("SB and arriving <3 min - Train: %d\n", i);
        //  using namespace std;

        // Find index of Stop ID
        for (int y = 0; y + 1 < SBarr_size;
             y++) // Loop through stopIDs (stations)
        {
          // Serial.printf("y: %d", y);
          if (nextStpId ==
              SB_station[0][y]) // If current value is equal to Stop ID
          {
            /*             Serial.printf("\n");
                        Serial.printf("LED Blink - %s", nextStationName);
                        Serial.printf(", index #: %d\n", y); */
            SB_station[1][y] = blink;
          }
        }
      } else {
        for (int y = 0; y + 1 < SBarr_size;
             y++) // Loop through stopIDs (stations)
        {
          /*           Serial.printf("y: %d", y); */
          if (nextStpId ==
              SB_station[0][y]) // If current value is equal to Stop ID
          {
            /*             Serial.printf("\n");
                        Serial.printf("LED Off - %s", nextStationName);
                        Serial.printf(", index #: %d\n", y); */
            SB_station[1][y] = off;
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

  /*   for (int i = 1; i < 2; ++i) {

      // Print SB_station array
      for (int j = 0; j < 33; ++j) {
        cout << "SB_station[" << i << "][" << j << "] = " << SB_station[i][j]
             << endl;
      }
    } */

  Stations_On();
  Stations_Blink();

  Serial.println("End Show_LED\n");
}
