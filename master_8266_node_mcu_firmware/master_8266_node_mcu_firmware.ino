#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

#define DEBUG_MODE  false
#define TFT_ENABLED true

#define WIFI_SSID   "MustangGT"
#define WIFI_PWD    "ignorepassword"

#define MQTT_SERVER "192.168.4.1"
#define MQTT_SERVER_PORT 1883
#define MQTT_TOPIC  "vehicle/lto/voltages"

#define SDA         D2
#define SCL         D1

#define TFT_CS      D0
#define TFT_RST     -1
#define TFT_DC      D3

#define BANKS_COUNT 2
#define CELL_COUNT  6

#define CELL_MEMORY_ALLOCATION 32
#define BANK_INFO_MEMORY_ALLOCATION 64

const int SINGLE_BANK_CAPACITY = CELL_MEMORY_ALLOCATION * CELL_COUNT;

// IMPORTANT: make sure MQTT_MAX_PACKET_SIZE >= TOTAL_BANKS_CAPACITY
// MQTT_MAX_PACKET_SIZE is definde in PubSubClient.h
const int TOTAL_BANKS_CAPACITY = (SINGLE_BANK_CAPACITY + BANK_INFO_MEMORY_ALLOCATION) * BANKS_COUNT;
const int BANK_DEVICE_ADDRESSES[] = {9, 10};

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

StaticJsonBuffer<TOTAL_BANKS_CAPACITY> banks_json_buffer;
JsonArray& banks_voltages = banks_json_buffer.createArray();

WiFiClient esp_client;
PubSubClient mqtt_client(esp_client);

void setup() {
  Wire.begin(SDA, SCL);

  initializeDebug();

  initializeBanks();  
  initializeWifi();
  initializeMqtt();
  initializeTFT();
}

void loop() {
  for(int bank_number = 0; bank_number < BANKS_COUNT; bank_number++) {
    processBank(bank_number);
  }

  processMqtt();  
  publishVoltages();
  
  debugSayTotal();
  tftSayTotal();

  delay(DEBUG_MODE ? 1000 : 200);
}

void publishVoltages() {
  char voltages_char[TOTAL_BANKS_CAPACITY];
  banks_voltages.printTo(voltages_char);
  mqtt_client.publish(MQTT_TOPIC, voltages_char);
}

void processBank(int bank_number) {
  int index = 0;
  char voltages_buffer [SINGLE_BANK_CAPACITY];
  DynamicJsonBuffer dynamic_json_buffer;

  debugSay("Request Bank ");
  debugSay(bank_number + 1);
  debugSay("... ");

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
            
    debugSayln(voltages);
  } else {
    debugSayln("Valtages parsing failed");
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
    debugSay("Attempting MQTT connection...");
    
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    
    // Attempt to connect
    if (mqtt_client.connect(clientId.c_str())) {
      debugSayln("connected");
    } else {
      debugSay("failed, rc=");
      debugSay(mqtt_client.state());
      debugSayln(" try again in 5 seconds");
      
      delay(5000);
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
    
  debugSay("\nConnecting to ");
  debugSayln(WIFI_SSID);

  WiFi.mode(WIFI_STA);  // station mode
  WiFi.begin(WIFI_SSID, WIFI_PWD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    debugSay(".");
  }

  randomSeed(micros());

  debugSayln("");
  debugSayln("WiFi connected");
  debugSayln("IP address: ");
  debugSayln(WiFi.localIP());
}

void initializeMqtt() {
  mqtt_client.setServer(MQTT_SERVER, MQTT_SERVER_PORT);

  // Define callback if needed (topic subscibe)
  //
  //mqtt_client.setCallback(callback); 
}

void initializeTFT() {
  if(TFT_ENABLED) {
    tft.initR(INITR_BLACKTAB);
    //tft.setTextWrap(false); // Allow text to run off right edge
    tft.fillScreen(ST77XX_BLACK);
    tft.setRotation(1);

    tftSayTotal();
  }
}

// ----- Initializers End ------

void tftSayTotal() {
  if(TFT_ENABLED) {
    tft.setCursor(0, 0);
    tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
    tft.setTextSize(1);

    for(int bank_number = 0; bank_number < BANKS_COUNT; bank_number++) {
      tft.println(banks_voltages[bank_number]["name"].as<String>());

      for(int cell = 0; cell < CELL_COUNT; cell++) {
        char cell_buf[4];

        if(cell == CELL_COUNT/2) {
           tft.println();
        }

        dtostrf(banks_voltages[bank_number]["voltages"][cell].as<float>(), 4, 2, cell_buf);
        tft.print(cell_buf);
        tft.print("v ");
      }

      tft.println("\n");
    }
  }
}

// DEBUG helper methods

void initializeDebug() {
  if(DEBUG_MODE) {
    Serial.begin(115200);
    debugSayln("Initialized Master.");
    debugSayTotal();
  }
}

void debugSayTotal() {
  if(DEBUG_MODE) {
    banks_voltages.printTo(Serial);
    Serial.println();
  }
}

void debugSay(char* message) {
  if(DEBUG_MODE) {
    Serial.print(message);
  }
}

void debugSay(int integer) {
  if(DEBUG_MODE) {
    Serial.print(integer);
  }
}

void debugSayln(char* message) {
  if(DEBUG_MODE) {
    Serial.println(message);
  }
}

void debugSayln(int integer) {
  if(DEBUG_MODE) {
    Serial.println(integer);
  }
}

void debugSayln(ArduinoJson::JsonArray& arr) {
  if(DEBUG_MODE) {
    arr.printTo(Serial);
    Serial.println();
  }
}
