#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <ArduinoJson.h>
#include <Wire.h>

#define DEBUG_MODE  false
#define TFT_ENABLED true

#define SDA         D1
#define SCL         D2

#define TFT_CS      D0
#define TFT_RST     -1
#define TFT_DC      D3

#define BANKS_COUNT 2
#define CELL_COUNT  6

const int BANK_DEVICE_ADDRESSES[] = {9, 10};
const int SINGLE_BANK_CAPACITY = JSON_ARRAY_SIZE(CELL_COUNT);
const int TOTAL_BANKS_CAPACITY = JSON_ARRAY_SIZE(BANKS_COUNT)
  + BANKS_COUNT * JSON_OBJECT_SIZE(BANKS_COUNT)
  + BANKS_COUNT * JSON_ARRAY_SIZE(CELL_COUNT+1);

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

StaticJsonBuffer<TOTAL_BANKS_CAPACITY> banks_json_buffer;
JsonArray& banks_voltages = banks_json_buffer.createArray();

void setup() {
  Wire.begin(SDA, SCL);

  initializeBanks();
  initializeTFT();
  initializeDebug();
}

void loop() {
  for(int bank_number = 0; bank_number < BANKS_COUNT; bank_number++) {
    processBank(bank_number);
  }

  debugSayTotal();
  tftSayTotal();

  if(DEBUG_MODE) {
    delay(1000);
  } else {
    delay(200);
  }  
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

void initializeBanks() {
  for(int index = 0; index < BANKS_COUNT; index++) {
    JsonObject& bank_obj = banks_json_buffer.createObject();
    JsonArray& bank_voltages = banks_json_buffer.createArray();

    for(int cell = 0; cell < CELL_COUNT; cell++) {
      bank_voltages.add(0.0);
    }
    
    bank_obj["name"] = String("Bank ") + (index + 1);    
    bank_obj["voltages"] = bank_voltages;
    banks_voltages.add(bank_obj);
  }
}

void processBank(int bank_number) {
  int index = 0;
  char voltages_buffer [SINGLE_BANK_CAPACITY];

  debugSay("Request Bank ");
  debugSay(bank_number + 1);
  debugSay("... ");
  
  Wire.requestFrom(BANK_DEVICE_ADDRESSES[bank_number], SINGLE_BANK_CAPACITY);

  while (Wire.available()) {  
    voltages_buffer[index++] = Wire.read();
  }

  DynamicJsonBuffer dynamic_json_buffer;
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

      tft.println();
      tft.println();
    }
  }
}

// DEBUG helper methods

void initializeDebug() {
  if(DEBUG_MODE) {
    Serial.begin(9600);
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
