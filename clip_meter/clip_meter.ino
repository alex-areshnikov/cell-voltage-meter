#include "DebugHelper.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define DEBUG_MODE false 

#define WIFI_SSID   "MustangGT"
#define WIFI_PWD    "ignorepassword"

#define MQTT_SERVER             "192.168.4.1"
#define MQTT_SERVER_PORT        1883
#define MQTT_CLIPPING_TOPIC     "vehicle/monoblock/clipping"

WiFiClient esp_client;
PubSubClient mqtt_client(esp_client);

DebugHelper debug(DEBUG_MODE);

void setup() {
  debug.initialize(115200);

  initializeWifi();
  initializeMqtt();
}

// the loop function runs over and over again forever
void loop() {
  processMqtt();

  int raw_reading = analogRead(0);

  debug.sayln(raw_reading);

  char buffer[16];
  itoa(raw_reading, buffer, 10);
  mqtt_client.publish(MQTT_CLIPPING_TOPIC, buffer);

  delay(100);
}

void initializeWifi() {
  delay(10);
    
  debug.say("\nConnecting to ");
  debug.sayln(WIFI_SSID);

  WiFi.mode(WIFI_STA);  // station mode
  WiFi.begin(WIFI_SSID, WIFI_PWD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    debug.say(".");
  }

  randomSeed(micros());

  debug.sayln("");
  debug.sayln("WiFi connected");
  // debug.sayln("IP address: ");
  // debug.sayln(WiFi.localIP());
}

void processMqtt() {
  if (!mqtt_client.connected()) {
    mqtt_reconnect();
  }
  
  mqtt_client.loop();
}

void initializeMqtt() {
  mqtt_client.setServer(MQTT_SERVER, MQTT_SERVER_PORT);
  mqtt_client.setCallback(mqtt_callback); 
}

void mqtt_reconnect() {
  while (!mqtt_client.connected()) {
    debug.say("Attempting MQTT connection...");
    
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    
    // Attempt to connect
    if (mqtt_client.connect(clientId.c_str())) {
      debug.sayln("connected");
    } else {
      debug.say("failed, rc=");
      debug.say(mqtt_client.state());
      debug.sayln(" try again in 5 seconds");
      
      delay(5000);
    }
  }
}

void mqtt_callback(char* topic, byte* payload, unsigned int length)
{
  debug.sayln("MQTT message received.");
  debug.say("Topic: ");
  debug.sayln(topic);
  debug.say("Payload: ");    
  debug.sayln((char*)payload);

  // TODO: Describe receive logic
}