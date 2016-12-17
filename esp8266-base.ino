////////////
// Base code for ESP8266 for Sean's Home Automation
// Does WifiManager, MQTT, and OTA Updates 
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
#define REFORMAT 0
// Compressed everything into this .h file, to clean up the actual function
//#include "esp8266basefunctions.h"
// Functions and variables used in the Wifi 
void loopBaseFunctions();
void setupBaseFunctions();
void reconnect();
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// #wifimanager - these are offered on WifiManager 
char mqtt_server[20];
char mqtt_port[6] = "1883";
char mqtt_topic[41] = "home/esp/kitchen";  // allow 40 char max length

//flag for saving data
bool shouldSaveConfig = false;
void MQTTcallback(char*, byte*, unsigned int);
WiFiClient espClient;
PubSubClient client(espClient);

// this will hold the hostname, like esp8266-123456 
char host[20] = "esp8266-test"; 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
void setup() {
  // the very FIRST thing to do is turn the light on  
  setupBaseFunctions(); // <-- must be FIRST
  // at this point you're connected, MQTT is working serial is enabled 115200
  delay(1000);
  //Consider MQTT Publish here to notify HA that we're now online (and in our startup state)
}

void MQTTcallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  char message[length + 1];
  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';
  Serial.println(message);

  // Add Code here to handle MQTT Messages
}

/* 
 // Consider an MQTT Response (Required by HomeAssistant)  
 
void sendState() {
  StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["state"] = (stateOn) ? on_cmd : off_cmd;
  JsonObject& color = root.createNestedObject("color");
  color["r"] = red;
  color["g"] = green;
  color["b"] = blue;
  root["brightness"] = brightness;
  char buffer[root.measureLength() + 1];
  root.printTo(buffer, sizeof(buffer));
  client.publish(mqtt_topic, buffer, true);
}
*/


void loop() {



  
  
  //run at the end of each cycle:
  loopBaseFunctions();
}



//WifiManager will call this to save configuration
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(host)) {
      Serial.println("connected");
      client.publish("esp8266boot","connected");
      char subbuf[44]; // 40 + \0 + the number of characters we're concatenating below
      snprintf(subbuf, 44, "%s/%s", mqtt_topic, "set"); 
      client.subscribe(subbuf);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }

}

ESP8266WebServer server(80);
const char* serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";


void setupBaseFunctions() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();

  sprintf(host, "esp8266-%d", ESP.getChipId());

  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    //clean FS, for testing
    //
    #if REFORMAT == 1
    SPIFFS.format();              // this will forget all custom params (MQTT)
    #endif
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(mqtt_server, json["mqtt_server"]);
          strcpy(mqtt_port, json["mqtt_port"]);
          strcpy(mqtt_topic, json["mqtt_topic"]);

        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  WiFiManagerParameter custom_mqtt_server("server", "192.168.140.14", mqtt_server, 20);
  WiFiManagerParameter custom_mqtt_port("port", "1883", mqtt_port, 5);
  WiFiManagerParameter custom_mqtt_subscribe("subscribe", "home/esp/kitchen/set", mqtt_topic, 40);
  WiFiManager wifiManager;

  #if REFORMAT == 1
  wifiManager.resetSettings();  // this will forget wifi/pass 
  #endif

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  //add all your parameters here
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_subscribe);


  if (!wifiManager.autoConnect()) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("Connected to WIFI");

  //read updated parameters
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(mqtt_topic, custom_mqtt_subscribe.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["mqtt_server"] = mqtt_server;
    json["mqtt_port"] = mqtt_port;
    json["mqtt_topic"] = mqtt_topic;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  client.setServer(mqtt_server, atoi(mqtt_port));
  client.setCallback(MQTTcallback);

  if (WiFi.waitForConnectResult() == WL_CONNECTED) {
    MDNS.begin(host);
    server.on("/", HTTP_GET, []() {
      server.sendHeader("Connection", "close");
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(200, "text/html", serverIndex);
    });
    server.on("/update", HTTP_POST, []() {
      server.sendHeader("Connection", "close");
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
      ESP.restart();
    }, []() {
      HTTPUpload& upload = server.upload();
      if (upload.status == UPLOAD_FILE_START) {
        Serial.setDebugOutput(true);
        WiFiUDP::stopAll();
        Serial.printf("Update: %s\n", upload.filename.c_str());
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if (!Update.begin(maxSketchSpace)) { //start with max available size
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) { //true to set the size to the current progress
          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
          Update.printError(Serial);
        }
        Serial.setDebugOutput(false);
      }
      yield();
    });
    server.begin();
    MDNS.addService("http", "tcp", 80);
    Serial.printf("Ready! Open http://%s.local in your browser\n", host);
  } else {
    Serial.println("WiFi Failed");
  }
  // send it through a loop of the normal run to connect and get MQTT working before proceeding
  loopBaseFunctions();
  loopBaseFunctions();
  Serial.printf("Base heap size: %u\n", ESP.getFreeHeap());
}

void loopBaseFunctions() {
  // This is the PubSubClient
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  // This is for the firmware OTA
  server.handleClient();
  
}


