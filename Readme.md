ESP8266-base
============

This code serves as my starting point for any ESP8266 projects (either on Wemos, Sonoff, MagicLight, etc). In itself, this code doesn't do much, after flashing the ESP8266 will become a wifi hotspot and allow for configuration, however most of the magic happens in code you need to write on top of this. 

This was written in Arduino 1.6.5 with the ESP8266 libraries installed.

Features
========
1. OTA Firmware Updates
2. TZapu's WifiManager
3. PubSub Client

Installation
============

If you've never worked with the Wemos before and are using OSX/Windows, then you'll need to download the CH340 USB->Serial driver for your OS. Google to find the appropriate driver. Most other cheap ESP8266 boards are using this chip as well, so this will probably be required if you're using the NodeMCU or other board.

## Setup Arduino

Out of the box, Arduino is missing a compiler for the ESP8266, as well as some libraries which we will use later. 

1. In Arduino, under Edit -> Preferences add the following URL to the "Additional Boards Manager URLs" field: 
http://arduino.esp8266.com/stable/package_esp8266com_index.json

2. Under Tools → Board → Boards Manager search for ESP8266 and click Install

3. In the Library Manager (Sketch → Include Library → Library Manager), search for and install the latest of: 

    1. PubSubClient
    2. ArduinoJSON
    3. (Probably useful: Infrared remote libraries)

4. Download Ken Taylor's WifiManager from https://github.com/kentaylor/WiFiManager and extract it to the Arduino Libraries folder

## Happy Coding

With the above libraries and configuration, you should be able to upload + run this code on the ESP8266; You will need to adjust the ```setup()``` and ```loop()``` functions to actually do what you want to accomplish. Check out some of my branches which I use for various lights or sensornodes for inspiration.
