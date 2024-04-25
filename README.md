# CTA-Train-Tracker
This project uses an ESP32 to pull CTA train data from the Train Tracker API provided by the CTA and uses the data to visualize southbound redline trains on an LED strip.


![Sample Video](https://github.com/cyandek/CTA-Train-Tracker/assets/158543011/0e763334-79be-4d1b-958a-f04830bb0f2d)

I printed out my own map to lay over the LED strip (the printer cut off the 95th and North/Clybourn stops). The LEDs will turn on when a southbound train arrives at a station and the LEDs will blink when a train is arriving in less than three minutes. The data is refreshed every 15 seconds. 

CTA Train Tracker API
https://www.transitchicago.com/developers/ttdocs/

Apply for API key
Add key to API_URL in secrets.h
Download the Espressif Esptouch app to configure the WiFi or update the WiFi SSID and password in secrets.h, uncomment the 'WiFi_Hardcoded' function in main.cpp and comment the 'WiFi_SmartConfig' in main.cpp

Flash to ESP32

I'm powering the ESP32 and LED strip with a 5V 10A DC power supply (could get away with less amps but its what I had laying around). 
![ESP32 Wiring 1](https://github.com/cyandek/CTA-Train-Tracker/assets/158543011/eb11054e-752a-485e-87bc-d00ead1026d0)

I split the +/- wires from the power supply to power both with the same PSU. You could power them separately, but I believe they would need to share a ground wire.

![ESP32 Wiring 2](https://github.com/cyandek/CTA-Train-Tracker/assets/158543011/ffb1f16d-1aca-4a00-9ac6-0f3a6c41a5ef)

Connect data wire to led strip (WS2812B or something similar cut to length of 33) using data pin 4.

![LED Wiring](https://github.com/cyandek/CTA-Train-Tracker/assets/158543011/b734f8f3-260b-4431-b6b4-bf70eb9b7fe2)
