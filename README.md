# CTA-Train-Tracker
Pull CTA train data from the Train Tracker API provided by the CTA and use the data to visualize southbound redline trains on an LED strip.


CTA Train Tracker API
https://www.transitchicago.com/developers/ttdocs/


Apply for API key
Add key to API_URL and update the WiFi SSID and password in secrets.h
Flash to esp32
Connect data wire to led strip (WS2812B or something similar cut to length of 33) using data pin 4.