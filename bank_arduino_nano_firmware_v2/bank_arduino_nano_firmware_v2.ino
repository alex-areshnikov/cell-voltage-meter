#include <Arduino.h>

const float BOARD_VOLTAGE = 5.01;
const int   VOLTAGE_PINS[] = {0,1,2,3,6,7};
const float VOLTAGE_CORRECTIONS[] = {0, 0.03, 0, 0, 0.13, 0};
const int   REDUCTION_FACTOR_MULTIPLIERS[] = {1,2,2,3,3,4};

float pin_read;

typedef union {
 float value;
 byte binary[4];
} voltageContainer;

voltageContainer voltage;

void setup() {
  Serial.begin(115200);
}

void loop() {
  for(byte index = 0; index < 6; index++) {
    pin_read = analogRead(VOLTAGE_PINS[index]);

    float calculated_voltage = calculateVoltageFor(pin_read, index);
    voltage.value = roundf(calculated_voltage * 100) / 100.0;

    Serial.write(0xFF);
    Serial.write(index);
    Serial.write(voltage.binary, 4);
    
    delay(30);
  } 
}

float calculateVoltageFor(float pin_read, int cell_number) {
  int multiplier = REDUCTION_FACTOR_MULTIPLIERS[cell_number];
  float correction = VOLTAGE_CORRECTIONS[cell_number];
  return (pin_read * ((BOARD_VOLTAGE * multiplier) / 1023) + correction);
}
