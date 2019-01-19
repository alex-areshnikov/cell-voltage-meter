#include <Arduino.h>
#include <ArduinoJson.h>
#include <Wire.h>
// #include "DebugHelper.h"
#include "DebugScreen.h"

#define DEBUG_MODE true

const float BOARD_VOLTAGE = 5.01;
const int   DEVICE_ADDRESS = 9;
const int   VOLTAGE_PINS[] = {0,1,2,3,6,7};
const float VOLTAGE_CORRECTIONS[] = {0, 0, 0, 0, 0.13, 0};
const int   REDUCTION_FACTOR_MULTIPLIERS[] = {1,2,2,3,3,4};

const int CAPACITY = JSON_ARRAY_SIZE(6);

char voltages_buffer[CAPACITY];

StaticJsonBuffer<CAPACITY> json_buffer;

float pin_read;
float voltage;

int line;

DebugScreen debug(DEBUG_MODE);

void setup() {
  Wire.begin(DEVICE_ADDRESS);
  Wire.onRequest(requestEvent);

  // debug.initialize(9600);  
  debug.initialize();  
}

void loop() {
  json_buffer.clear();
  JsonArray& voltages = json_buffer.createArray();
  line = 0;

  
  
  for(int index = 0; index < 6; index++) {
    pin_read = analogRead(VOLTAGE_PINS[index]);
    voltage = calculateVoltageFor(pin_read, index);

    voltage = roundf(voltage * 100) / 100.0;
    voltages.add(voltage);

    if(index % 2) {
      debug.set_line(line++);
      debug.sayn(voltage);
      debug.say(" ");
    } else {
      debug.sayln(voltage);
    }    
  }

  voltages.printTo(voltages_buffer);
  // debug.sayn(voltages_buffer);

  delay(200); 
}

float calculateVoltageFor(float pin_read, int cell_number) {
  int multiplier = REDUCTION_FACTOR_MULTIPLIERS[cell_number];
  float correction = VOLTAGE_CORRECTIONS[cell_number];
  return (pin_read * ((BOARD_VOLTAGE * multiplier) / 1023) + correction);
}

void requestEvent() {
  Wire.write(voltages_buffer);
}
