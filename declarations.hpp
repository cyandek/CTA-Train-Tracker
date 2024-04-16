#include <ArduinoJson.h>
#include <FastLED.h>
#include <HTTPClient.h>
#include <WiFiMulti.h>
#include <string>

// Delarations for LED strip
#define NUM_LEDS 33
#define DATA_PIN 4
#define FASTLED_INTERNAL

CRGB leds[NUM_LEDS] = {0};

// Interval & time variables
int APIinterval = 15000;                        //API call every 15 secs
int blinkinterval = 2000;                       //blink LEDs every second

static unsigned long msLastAPI = millis();      //time keeper for API calls
static unsigned long msLastThread = millis();      //dont think I'm using this one

// Set northbound/southbound train direction variables for redline trains
const int NBRed = 1;
const int SBRed = 5;

// Create arrays for southbound/northbound Stop IDs for LED strip

int SBarr[33] = {30089, 30276, 30047, 30192, 30178, 30224, 30238, 30037, 30194, 30270, 30286, 30110, 30212, 30290, 30065, 30280, 30122, 30126, 30234, 30256, 30274, 30017, 30106, 30150, 30230, 30067, 30268, 30170, 30148, 30252, 30021, 30228, 30174};
int SBarr_size = sizeof(SBarr) / sizeof(SBarr[0]); // the size of array

//2D array for southbound stop IDs and station status 
 int SB_station[2][33] = {
    {30089, 30276, 30047, 30192, 30178, 30224, 30238, 30037, 30194, 30270, 30286, 30110, 30212, 30290, 30065, 30280, 30122, 30126, 30234, 30256, 30274, 30017, 30106, 30150, 30230, 30067, 30268, 30170, 30148, 30252, 30021, 30228, 30174},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

//0 = no train = LED off
//1 = train approaching platform = LED on
//2 = train arriving in <3 = LED blink at LED interval


int NBarr[33] = {30088, 30275, 30046, 30191, 30177, 30223, 30237,
                 30036, 30193, 30269, 30285, 30109, 30211, 30289,
                 30064, 30279, 30121, 30125, 30233, 30255, 30273,
                 30016, 30105, 30149, 30229, 30066, 30267, 30169,
                 30147, 30251, 30020, 30227, 30173};
int NBarr_size = sizeof(NBarr) / sizeof(NBarr[0]); // the size of array

WiFiMulti wifiMulti;


// HTTPClient for HTTP requests
WiFiClient client;
HTTPClient http;

//Create json document and pointer to pass to threads
JsonDocument doc;
JsonDocument *doc_ptr = &doc;

//Declare json variables
int direction, isApp, nextStpId;
const char *nextStationName;
const char *arrT;
const char *prdt;

//Helpers
int i,y = 0;

int off = 0;
int on = 1;
int blink = 2;

int pulseType = 0;     // On/off switch for LED_Blink function
bool available = false; //mutex 
