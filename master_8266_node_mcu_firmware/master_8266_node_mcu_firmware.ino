#include <ArduinoJson.h>
#include <Wire.h>

const int BANK_DEVICE_ADDRESSES[] = {9, 10};
const int CAPACITY = JSON_ARRAY_SIZE(6);

void setup() {
  Wire.begin(D2, D1);

  Serial.begin(9600);
  Serial.println("Initialized Master.");
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

  Serial.print("\nRequest Bank ");
  Serial.println(bank_number);

  Wire.requestFrom(BANK_DEVICE_ADDRESSES[bank_number-1], CAPACITY);

  while (Wire.available()) {  
    char chr = Wire.read();
    voltages_buffer[index] = chr;
    index++;
  }

  JsonArray& voltages = json_buffer.parseArray(voltages_buffer);

  if (voltages.success()) {
    for(int i=0; i<6; i++) {
      Serial.print("Cell ");
      Serial.print(i+1);
      Serial.print(": ");
      Serial.print(voltages[i].as<float>());
      Serial.print("v ");
    }
    Serial.println("\n------");
  } else {
    Serial.println("parseObject() failed");
  }  
}
