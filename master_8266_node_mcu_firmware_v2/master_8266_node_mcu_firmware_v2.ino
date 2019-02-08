#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DebugHelper.h"

#define DEBUG_MODE  false 

#define WIFI_SSID   "MustangGT"
#define WIFI_PWD    "ignorepassword"

#define MQTT_SERVER           "192.168.4.1"
#define MQTT_SERVER_PORT      1883
#define MQTT_VOLTAGES_TOPIC   "vehicle/lto/voltages"
#define MQTT_CHARGE_TOPIC     "vehicle/lto/charge"
#define MQTT_KEEP_ALIVE_TOPIC "vehicle/lto/keep_alive"

#define RECEIVING_START_BYTE 0
#define RECEIVING_CELL_ID 1
#define RECEIVING_VOLTAGE 2

// WeMos GPIO
// #define D0 16 // 3
// #define D1 5 // 1
// #define D2 4 // 16
// #define D3 0 // 5
// #define D4 2 // 4
// #define D5 14 // 14
// #define D6 12 // 12
// #define D7 13 // 13
// #define D8 15 // 0

// #define SDA         D2
// #define SCL         D1

#define CHARGE_RELAY_PIN D3

#define CELL_COUNT  6
#define BANK_NAME "Bank 1"

// IMPORTANT: make sure MQTT_MAX_PACKET_SIZE >= TOTAL_BANKS_CAPACITY
// MQTT_MAX_PACKET_SIZE is definde in PubSubClient.h
// const int TOTAL_BANKS_CAPACITY = (SINGLE_BANK_CAPACITY + BANK_INFO_MEMORY_ALLOCATION) * BANKS_COUNT;

const int BANK_DEVICE_ADDRESS = 9;
const int CELLS_CAPACITY = JSON_ARRAY_SIZE(CELL_COUNT);
const int TOTAL_BANK_CAPACITY = CELLS_CAPACITY + JSON_OBJECT_SIZE(2);

StaticJsonBuffer<TOTAL_BANK_CAPACITY> bank_json_buffer;
JsonArray& voltages = bank_json_buffer.createArray();
JsonObject& bank_voltages = bank_json_buffer.createObject();

WiFiClient esp_client;
PubSubClient mqtt_client(esp_client);

DebugHelper debug(DEBUG_MODE);

typedef union {
 float value;
 byte binary[4];
} voltageContainer;

voltageContainer voltage;

int cell_id;

uint8_t counter;
uint8_t state;

void setup() {
  initializeBoard();

  initializeBanks();  
  initializeWifi();
  initializeMqtt();

  state = RECEIVING_START_BYTE;

  debug.sayln('Initialized.');
}

void loop() {
  processSerial();
  processMqtt(); 
}

void publishVoltages() {
  char bank_buffer[TOTAL_BANK_CAPACITY];
  bank_voltages.printTo(bank_buffer);
  mqtt_client.publish(MQTT_VOLTAGES_TOPIC, bank_buffer);
}

void processSerial() {
  if(!Serial.available()) {
    return;
  }

  byte incoming_byte = Serial.read();

  if(state == RECEIVING_START_BYTE && incoming_byte == 0xFF) {
    state = RECEIVING_CELL_ID;
    return;
  }

  if(state == RECEIVING_CELL_ID) {
    cell_id = incoming_byte;
    debug.say(" [");
    debug.say(cell_id);
    debug.say("]");
    state = RECEIVING_VOLTAGE;
    counter = 0;
    return;
  }
  
  if(state == RECEIVING_VOLTAGE) {
    voltage.binary[counter++] = incoming_byte;
  }

  if(state == RECEIVING_VOLTAGE && counter == sizeof(float)) {
    state = RECEIVING_START_BYTE;
    voltages[cell_id] = voltage.value;

    debug.say(voltage.value);

    if(cell_id+1 == CELL_COUNT) {
      publishVoltages();
      debug.sayln("");
    }
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

      if(mqtt_client.subscribe(MQTT_KEEP_ALIVE_TOPIC)) {
        debug.say("MQTT subscribed to ");
        debug.sayln(MQTT_KEEP_ALIVE_TOPIC);
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

  if(String(topic) == MQTT_KEEP_ALIVE_TOPIC) {
    if(String((char*)payload).startsWith("ping")) {
      mqtt_client.publish(MQTT_KEEP_ALIVE_TOPIC, "pong");
    }
  }
}

// ----- Initializers ------

void initializeBoard() {
  Serial.begin(115200);
  pinMode(CHARGE_RELAY_PIN, OUTPUT);  
  digitalWrite(CHARGE_RELAY_PIN, HIGH);
}

void initializeBanks() {
    for(int cell = 0; cell < CELL_COUNT; cell++) {
      voltages.add(0.0);
    }

    bank_voltages["name"] = BANK_NAME;
    bank_voltages["voltages"] = voltages;
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

void initializeMqtt() {
  mqtt_client.setServer(MQTT_SERVER, MQTT_SERVER_PORT);
  mqtt_client.setCallback(mqtt_callback); 
}