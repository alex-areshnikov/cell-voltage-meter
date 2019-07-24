#include <Arduino.h>
#include "DebugHelper.h"

DebugHelper::DebugHelper(bool debugMode) {
  this->debugMode = debugMode;  
}

void DebugHelper::initialize(int baud = 9600) {
  if(debugMode) {
    Serial.begin(baud);
    while (!Serial) continue;
  }
}

void DebugHelper::say(String message) {
  if(debugMode) {
    Serial.print(message);
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

void DebugHelper::sayln(char* message) {
  if(debugMode) {
    Serial.println(message);
  }
}

void DebugHelper::sayln(String message) {
  if(debugMode) {
    Serial.println(message);
  }
}

void DebugHelper::sayln(int integer) {
  if(debugMode) {
    Serial.println(integer);
  }
}