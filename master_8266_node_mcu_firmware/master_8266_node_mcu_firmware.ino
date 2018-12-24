#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

#include "DebugHelper.h"

#define DEBUG_MODE  false

#define WIFI_SSID   "MustangGT"
#define WIFI_PWD    "ignorepassword"

#define MQTT_SERVER           "192.168.4.1"
#define MQTT_SERVER_PORT      1883
#define MQTT_VOLTAGES_TOPIC   "vehicle/lto/voltages"
#define MQTT_CHARGE_TOPIC     "vehicle/lto/charge"

#define SDA         D2
#define SCL         D1

#define CHARGE_RELAY_PIN D3

#define BANKS_COUNT 2
#define CELL_COUNT  6

#define CELL_MEMORY_ALLOCATION 32
#define BANK_INFO_MEMORY_ALLOCATION 64

const int SINGLE_BANK_CAPACITY = CELL_MEMORY_ALLOCATION * CELL_COUNT;

// IMPORTANT: make sure MQTT_MAX_PACKET_SIZE >= TOTAL_BANKS_CAPACITY
// MQTT_MAX_PACKET_SIZE is definde in PubSubClient.h
const int TOTAL_BANKS_CAPACITY = (SINGLE_BANK_CAPACITY + BANK_INFO_MEMORY_ALLOCATION) * BANKS_COUNT;
const int BANK_DEVICE_ADDRESSES[] = {9, 10};

StaticJsonBuffer<TOTAL_BANKS_CAPACITY> banks_json_buffer;
JsonArray& banks_voltages = banks_json_buffer.createArray();

WiFiClient esp_client;
PubSubClient mqtt_client(esp_client);

DebugHelper debug(DEBUG_MODE);

void setup() {
  Wire.begin(SDA, SCL);
  
  debug.initialize();
  initializeBanks();  
  initializeWifi();
  initializeMqtt();

  pinMode(CHARGE_RELAY_PIN, OUTPUT);  

  debug.sayln("Initialized Master.");
  debug.sayln(banks_voltages);
}

void loop() {
  for(int bank_number = 0; bank_number < BANKS_COUNT; bank_number++) {
    processBank(bank_number);
  }

  processMqtt();  
  publishVoltages();
  
  debug.sayln(banks_voltages);
  delay(DEBUG_MODE ? 1000 : 200);
}

void publishVoltages() {
  char voltages_char[TOTAL_BANKS_CAPACITY];
  banks_voltages.printTo(voltages_char);
  mqtt_client.publish(MQTT_VOLTAGES_TOPIC, voltages_char);
}

void processBank(int bank_number) {
  int index = 0;
  char voltages_buffer [SINGLE_BANK_CAPACITY];
  DynamicJsonBuffer dynamic_json_buffer;

  debug.say("Request Bank ");
  debug.say(bank_number + 1);
  debug.say("... ");

  Wire.requestFrom(BANK_DEVICE_ADDRESSES[bank_number], SINGLE_BANK_CAPACITY);

  while (Wire.available()) {
    voltages_buffer[index++] = Wire.read();
  }

  JsonArray& voltages = dynamic_json_buffer.parseArray(voltages_buffer);

  if (voltages.success()) {
    for(int cell = 0; cell < CELL_COUNT; cell++) {
      float volt = voltages[cell].as<float>();
      banks_voltages[bank_number]["voltages"][cell] = volt;
    }
            
    debug.sayln(voltages);
  } else {
    debug.sayln("Valtages parsing failed");
  }  
}

void processMqtt() {
  if (!mqtt_client.connected()) {
    mqtt_reconnect();
  }
  
  mqtt_client.loop();
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

      if(mqtt_client.subscribe(MQTT_CHARGE_TOPIC)) {
        debug.say("MQTT subscribed to ");
        debug.sayln(MQTT_CHARGE_TOPIC);
      }

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

  if(String(topic) == MQTT_CHARGE_TOPIC) {
    char first_chr = ((char*)payload)[0];

    if(first_chr == '1') {
      digitalWrite(CHARGE_RELAY_PIN, HIGH);
    }

    if(first_chr == '0') {
      digitalWrite(CHARGE_RELAY_PIN, LOW);
    }
  }
}

// ----- Initializers ------

void initializeBanks() {
  for(int index = 0; index < BANKS_COUNT; index++) {
    JsonObject& bank_obj = banks_json_buffer.createObject();
    JsonArray& bank_voltages = banks_json_buffer.createArray();

    for(int cell = 0; cell < CELL_COUNT; cell++) {
      bank_voltages.add(19.99);
    }

    bank_obj["bank_number"] = index + 1;
    bank_obj["voltages"] = bank_voltages;
    banks_voltages.add(bank_obj);
  }
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
  debug.sayln("IP address: ");
  debug.sayln(WiFi.localIP());
}

void initializeMqtt() {
  mqtt_client.setServer(MQTT_SERVER, MQTT_SERVER_PORT);
  mqtt_client.setCallback(mqtt_callback); 
}