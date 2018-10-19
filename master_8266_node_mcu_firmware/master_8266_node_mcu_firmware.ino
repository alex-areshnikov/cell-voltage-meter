#include <ArduinoJson.h>
#include <Wire.h>

const bool DEBUG_MODE = false;
const int BANK_DEVICE_ADDRESSES[] = {9, 10};

const int CAPACITY = JSON_ARRAY_SIZE(6);

void setup() {
  Wire.begin(D2, D1);

  if(DEBUG_MODE) {
    Serial.begin(9600);
    Serial.println("Initialized Master.");
  }
}

void loop() {
  process_bank(1);
  process_bank(2);
  delay(2000);
}

void process_bank(int bank_number) {
  int index = 0;
  char voltages_buffer [CAPACITY];
  StaticJsonBuffer<CAPACITY> json_buffer;

  debug_say("\nRequest Bank ");
  debug_sayln(bank_number);
  
  Wire.requestFrom(BANK_DEVICE_ADDRESSES[bank_number-1], CAPACITY);

  while (Wire.available()) {  
    char chr = Wire.read();
    voltages_buffer[index] = chr;
    index++;
  }

  JsonArray& voltages = json_buffer.parseArray(voltages_buffer);

  if (voltages.success()) {
    for(int i=0; i<6; i++) {
      debug_say("Cell ");
      debug_say(i+1);
      debug_say(": ");
      debug_say(voltages[i].as<float>());
      debug_say("v ");
    }
    debug_sayln("\n------");
  } else {
    debug_sayln("Valtages parsing failed");
  }  
}

// DEBUG helper methods

void debug_say(char* message) {
  if(DEBUG_MODE) {
    Serial.print(message);
  }
}

void debug_say(int integer) {
  if(DEBUG_MODE) {
    Serial.print(integer);
  }
}

void debug_sayln(char* message) {
  if(DEBUG_MODE) {
    Serial.println(message);
  }
}

void debug_sayln(int integer) {
  if(DEBUG_MODE) {
    Serial.println(integer);
  }
}
