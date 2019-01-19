#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

#define WIFI_SSID   "MustangGT"
#define WIFI_PWD    "ignorepassword"

#define MQTT_SERVER           "192.168.4.1"
#define MQTT_SERVER_PORT      1883
#define MQTT_VOLTAGES_TOPIC   "vehicle/lto/voltages"
#define MQTT_CHARGE_TOPIC     "vehicle/lto/charge"

// WeMos GPIO
#define D0 16 // 3
#define D1 5 // 1
#define D2 4 // 16
#define D3 0 // 5
#define D4 2 // 4
#define D5 14 // 14
#define D6 12 // 12
#define D7 13 // 13
#define D8 15 // 0

#define SDA         D2
#define SCL         D1

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
JsonObject& bank_voltages = bank_json_buffer.createObject();

WiFiClient esp_client;
PubSubClient mqtt_client(esp_client);

void setup() {
  initializeBoard();
  
  initializeBanks();  
  initializeWifi();
  initializeMqtt();
}

void loop() {
  processBank();

  processMqtt();  
  publishVoltages();
  
  delay(200);
}

void publishVoltages() {
  char bank_buffer[TOTAL_BANK_CAPACITY];
  bank_voltages.printTo(bank_buffer);
  mqtt_client.publish(MQTT_VOLTAGES_TOPIC, bank_buffer);
}

void processBank() {
  int index = 0;
  char voltages_buffer [CELLS_CAPACITY];
  DynamicJsonBuffer dynamic_json_buffer;

  Wire.requestFrom(BANK_DEVICE_ADDRESS, CELLS_CAPACITY);

  while (Wire.available()) {
    voltages_buffer[index++] = Wire.read();
  }

  JsonArray& voltages = dynamic_json_buffer.parseArray(voltages_buffer);

  if (voltages.success()) {
    for(int cell = 0; cell < CELL_COUNT; cell++) {
      float volt = voltages[cell].as<float>();
      bank_voltages["voltages"][cell] = volt;
    }            
  } else {
    // "Valtages parsing failed"
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

void initializeBoard() {
  Wire.begin(SDA, SCL);
  pinMode(CHARGE_RELAY_PIN, OUTPUT);  
  digitalWrite(CHARGE_RELAY_PIN, HIGH);
}

void initializeBanks() {
    JsonArray& voltages = bank_json_buffer.createArray();

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
  debug.sayln("IP address: ");
  debug.sayln(WiFi.localIP());
}

void initializeMqtt() {
  mqtt_client.setServer(MQTT_SERVER, MQTT_SERVER_PORT);
  mqtt_client.setCallback(mqtt_callback); 
}