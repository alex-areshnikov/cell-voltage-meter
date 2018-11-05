#include <Arduino.h>
#include <ArduinoJson.h>
#include "DebugHelper.h"

DebugHelper::DebugHelper(bool debugMode) {
  this->debugMode = debugMode;  
}

void DebugHelper::initialize() {
  if(debugMode) {
    Serial.begin(115200);
    while (!Serial) continue;
  }
}

void DebugHelper::say(char* message) {
  if(debugMode) {
    Serial.print(message);
  }
}

void DebugHelper::say(int integer) {
  if(debugMode) {
    Serial.print(integer);
  }
}

void DebugHelper::say(ArduinoJson::JsonArray& arr) {
  if(debugMode) {
    arr.printTo(Serial);
  }
}

void DebugHelper::sayln(char* message) {
  if(debugMode) {
    Serial.println(message);
  }
}

void DebugHelper::sayln(int integer) {
  if(debugMode) {
    Serial.println(integer);
  }
}

void DebugHelper::sayln(ArduinoJson::JsonArray& arr) {
  if(debugMode) {
    arr.printTo(Serial);
    Serial.println();
  }
}