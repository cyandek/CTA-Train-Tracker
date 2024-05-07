#include <ArduinoJson.h>
#include <FastLED.h>
#include <HTTPClient.h>
#include <WiFiMulti.h>
#include <string>

// Delarations for LED strip
#define NUM_LEDS 66
#define DATA_PIN 4
#define FASTLED_INTERNAL

WiFiMulti wifiMulti;

// HTTPClient for HTTP requests
WiFiClient client;
HTTPClient http;

CRGB leds[NUM_LEDS] = {0};

int fadeAmount =
    245; // Can be used in the blink function(instead of completely turning an
         // LED off this will dim it to near 0 brightness)

// Interval & time variables
int APIinterval = 15000;  // API call every 15 secs
int blinkinterval = 1000; // blink LEDs every second

static unsigned long msLastAPI = millis(); // time keeper for API calls

// Set northbound/southbound train direction variables for redline trains
const int NBRed = 1;
const int SBRed = 5;

// Array of the station names from South to North
const char *Station_names[33] = {
    "95th/Dan Ryan", "87th",           "79th",
    "69th",          "63rd",           "Garfield",
    "47th",          "Sox-35th",       "Cermak-Chinatown",
    "Roosevelt",     "Harrison",       "Jackson",
    "Monroe",        "Lake",           "Grand",
    "Chicago",       "Clark/Division", "North/Clybourn",
    "Fullerton",     "Belmont",        "Addison",
    "Sheridan",      "Wilson",         "Lawrence",
    "Argyle",        "Berwyn",         "Bryn Mawr",
    "Thorndale",     "Granville",      "Loyola",
    "Morse",         "Jarvis",         "Howard"};

// Array of Southbound StopIDs (South to North)
int SBarr[33] = {30089, 30276, 30047, 30192, 30178, 30224, 30238, 30037, 30194,
                 30270, 30286, 30110, 30212, 30290, 30065, 30280, 30122, 30126,
                 30234, 30256, 30274, 30017, 30106, 30150, 30230, 30067, 30268,
                 30170, 30148, 30252, 30021, 30228, 30174};

int SBarr_size = sizeof(SBarr) / sizeof(SBarr[0]); // the size of array = 33

// 2D Array of the Southbound StopIds and their status (South to North)
int SB_station[2][33] = {{30089, 30276, 30047, 30192, 30178, 30224, 30238,
                          30037, 30194, 30270, 30286, 30110, 30212, 30290,
                          30065, 30280, 30122, 30126, 30234, 30256, 30274,
                          30017, 30106, 30150, 30230, 30067, 30268, 30170,
                          30148, 30252, 30021, 30228, 30174},
                         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

// Array of Northbound StopIDs (South to North)
int NBarr[33] = {30088, 30275, 30046, 30191, 30177, 30223, 30237, 30036, 30193,
                 30269, 30285, 30109, 30211, 30289, 30064, 30279, 30121, 30125,
                 30233, 30255, 30273, 30016, 30105, 30149, 30229, 30066, 30267,
                 30169, 30147, 30251, 30020, 30227, 30173};

// 2D Array of the Northbound StopIds and their status (South to North)
int NB_station[2][33] = {{30088, 30275, 30046, 30191, 30177, 30223, 30237,
                          30036, 30193, 30269, 30285, 30109, 30211, 30289,
                          30064, 30279, 30121, 30125, 30233, 30255, 30273,
                          30016, 30105, 30149, 30229, 30066, 30267, 30169,
                          30147, 30251, 30020, 30227, 30173},
                         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

int NBarr_size =
    sizeof(NBarr) / sizeof(NBarr[0]); // the size of the NB array = 33

// Create json document and pointer to pass to functions
JsonDocument doc;
JsonDocument *doc_ptr = &doc;

// Declare json variables
int direction, isApp, nextStpId;
const char *nextStationName;
const char *arrT;
const char *prdt;

int i, y = 0;  // Helpers for loops
int off = 0;   // set the status of the stations in the 2D arrays to off
int on = 1;    // set the status of the stations in the 2D arrays to on
int blink = 2; // set the status of the stations in the 2D arrays to blink

// On/off switch for SB trains in LED_Blink function (South to North)
int SB_pulse[33] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// On/off switch for NB trains in LED_Blink function (South to North)
int NB_pulse[33] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

bool available =
    false; // mutex to lock other functions while API_Call is running
const bool debug =
    false; // switch to turn on most of the print functions for debugging
