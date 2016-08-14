// base working set: 
// - WifiManager
// - PubSubClient
// - OTA Firmware Upgrade 
// - reset button

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// For wifimanager
#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// for pubsub
#include <SPI.h>
#include <PubSubClient.h>
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// for OTA
#include <WiFiClient.h>
//#include <ESP8266WebServer.h> // <-- this is needed for both WifiManager and OTA; included above
#include <ESP8266mDNS.h>
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Compressed everything into this .h file, to clean up the actual function
#include "esp8266basefunctions.h"


void setup() {
  setupBaseFunctions(); // <-- must be FIRST 
  // at this point you're connected, MQTT is working serial is enabled 115200
  
}

void loop() {
  // add code here

  //run at the end of each cycle: 
  loopBaseFunctions();
}
