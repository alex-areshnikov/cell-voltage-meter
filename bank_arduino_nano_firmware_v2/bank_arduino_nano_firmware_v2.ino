#include <Arduino.h>

#define CHARGE_RELAY_PIN PD3
#define TRUNK_LIGHTS_PIN PD4

uint8_t CHARGE_ID = 1;
uint8_t LIGHTS_ID = 2;

const float BOARD_VOLTAGE = 5.02;
const int   VOLTAGE_PINS[] = {0,1,2,3,6,7};
const float VOLTAGE_CORRECTIONS[] = {0, 0, 0, 0, 0.1, 0};
const int   REDUCTION_FACTOR_MULTIPLIERS[] = {1,2,2,3,3,4};

float pin_read;

typedef union {
 float value;
 byte binary[4];
} voltageContainer;

voltageContainer voltage;

void setup() {
  Serial.begin(115200);

  pinMode(CHARGE_RELAY_PIN, OUTPUT);  
  digitalWrite(CHARGE_RELAY_PIN, HIGH);

  pinMode(TRUNK_LIGHTS_PIN, OUTPUT);  
  digitalWrite(TRUNK_LIGHTS_PIN, HIGH);
}

void loop() {
  processSerial();

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

void processSerial() {
  if(!Serial.available()) {
    return;
  }

  if(Serial.read() != 0xFA) {
    return;
  }

  uint8_t the_pin;  
  uint8_t object_id = Serial.read();
  uint8_t pin_value = Serial.read();

  if(object_id == CHARGE_ID) {
    the_pin = CHARGE_RELAY_PIN;
  }

  if(object_id == LIGHTS_ID) {
    the_pin = TRUNK_LIGHTS_PIN;
  }

  if(the_pin) {
    digitalWrite(the_pin, pin_value);
  }
}

float calculateVoltageFor(float pin_read, int cell_number) {
  int multiplier = REDUCTION_FACTOR_MULTIPLIERS[cell_number];
  float correction = VOLTAGE_CORRECTIONS[cell_number];
  return (pin_read * ((BOARD_VOLTAGE * multiplier) / 1023) + correction);
}
