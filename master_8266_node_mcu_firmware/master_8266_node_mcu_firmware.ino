#include <ArduinoJson.h>
#include <Wire.h>

const bool DEBUG_MODE = false;
const int BANK_DEVICE_ADDRESSES[] = {9, 10};
const int BANKS_COUNT = 2;
const int CELL_COUNT = 6;

const int SINGLE_BANK_CAPACITY = JSON_ARRAY_SIZE(CELL_COUNT);
const int TOTAL_BANKS_CAPACITY = JSON_ARRAY_SIZE(BANKS_COUNT)
  + BANKS_COUNT * JSON_OBJECT_SIZE(BANKS_COUNT)
  + BANKS_COUNT * JSON_ARRAY_SIZE(CELL_COUNT+1);

StaticJsonBuffer<TOTAL_BANKS_CAPACITY> banks_json_buffer;
JsonArray& banks_voltages = banks_json_buffer.createArray();

void setup() {
  Wire.begin(D2, D1);

  initializeBanks();

  if(DEBUG_MODE) {
    Serial.begin(9600);
    Serial.println("Initialized Master.");
    debugSayTotal();
  }
}

void loop() {
  for(int bank_number = 0; bank_number < BANKS_COUNT; bank_number++) {
    processBank(bank_number);
  }

  if(DEBUG_MODE) {
    delay(1000);
  } else {
    delay(200);
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
  StaticJsonBuffer<SINGLE_BANK_CAPACITY> json_buffer;

  debugSay("\nRequest Bank ");
  debugSayln(bank_number + 1);
  
  Wire.requestFrom(BANK_DEVICE_ADDRESSES[bank_number], SINGLE_BANK_CAPACITY);

  while (Wire.available()) {  
    voltages_buffer[index++] = Wire.read();
  }

  JsonArray& voltages = json_buffer.parseArray(voltages_buffer);

  if (voltages.success()) {
    banks_voltages[bank_number]["voltages"] = voltages;
    debugSayTotal();
  } else {
    debugSayln("Valtages parsing failed");
  }  
}

// DEBUG helper methods

void debugSayTotal() {
  if(DEBUG_MODE) {
    banks_voltages.printTo(Serial);
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
