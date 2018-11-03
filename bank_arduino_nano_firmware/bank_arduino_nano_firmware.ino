#include <ArduinoJson.h>
#include <Wire.h>

const bool  DEBUG_MODE = false;
const int   DEVICE_ADDRESS = 9;
const int   VOLTAGE_PINS[] = {0,1,2,3,6,7};
const float VOLTAGE_CORRECTIONS[] = {-0.1,-0.18,-0.28,-0.35,-0.49,-0.53};
const int   REDUCTION_FACTOR_MULTIPLIERS[] = {1,2,2,3,3,4};

const int CAPACITY = JSON_ARRAY_SIZE(6);

char voltages_buffer [CAPACITY];

void setup() {
  Wire.begin(DEVICE_ADDRESS);
  Wire.onRequest(requestEvent);

  if(DEBUG_MODE) {
    Serial.begin(9600);
    while (!Serial) continue;
    Serial.println("Initialized.");
  }
}

void loop() {
  float pin_read;
  float voltage;
  StaticJsonBuffer<CAPACITY> json_buffer;
  JsonArray& voltages = json_buffer.createArray();

  for(int index = 0; index < 6; index++) {
    pin_read = analogRead(VOLTAGE_PINS[index]);
    voltage = calculateVoltageFor(pin_read, index);

    voltage = roundf(voltage * 100) / 100.0;
    voltages.add(voltage);
  }

  voltages.printTo(voltages_buffer);

  debugSayTotal();

  delay(DEBUG_MODE ? 1000 : 200); 
}

float calculateVoltageFor(float pin_read, int cell_number) {
  int multiplier = REDUCTION_FACTOR_MULTIPLIERS[cell_number];
  float correction = VOLTAGE_CORRECTIONS[cell_number];
  return (pin_read * ((5.0 * multiplier) / 1023) + correction);
}

void requestEvent() {
  Wire.write(voltages_buffer);
}

void debugSayTotal() {
  if(DEBUG_MODE) {
    Serial.println(voltages_buffer);
  }
}
