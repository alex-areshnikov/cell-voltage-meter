#include <ArduinoJson.h>
#include <Wire.h>
   
const int VOLTAGE_PINS[] = {0,1,2,3,6,7};
const int DEVICE_ADDRESS = 10;
const int CAPACITY = JSON_ARRAY_SIZE(6);
const float VOLTAGE_CORRECTION = 0.14;

char voltages_buffer [CAPACITY];

void setup() {  
  Wire.begin(DEVICE_ADDRESS);
  Wire.onRequest(request_event);

  Serial.begin(9600);
  while (!Serial) continue;
  Serial.println("Initialized.");
}

void loop() {  
  StaticJsonBuffer<CAPACITY> json_buffer;  
  JsonArray& voltages = json_buffer.createArray();

  for(int index = 0; index < 6; index++) {
    float pin_read = analogRead(VOLTAGE_PINS[index]);
    float voltage = (pin_read/1023*5.0) - VOLTAGE_CORRECTION;

    voltage = roundf(voltage * 100) / 100.0;    
    voltages.add(voltage);
  }

  voltages.printTo(voltages_buffer);
  
  delay(200);
}

void request_event() {   
  Wire.write(voltages_buffer);
}
