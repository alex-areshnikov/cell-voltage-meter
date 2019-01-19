#include <Arduino.h>
#include <ArduinoJson.h>
#include "DebugScreen.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


DebugScreen::DebugScreen(bool debugMode) {
  this->debugMode = debugMode;  

  if(!debugMode) { return; }

  for(int i=0; i<4; i++) {
    this->display_messages[i] = "";
  }
  
  this->display = new Adafruit_SSD1306(OLED_RESET);    
}

void DebugScreen::initialize(){
  if(!debugMode) { return; }

  display->begin();
  display->clearDisplay();
  display->setTextSize(1);

  set_line(0);
}

void DebugScreen::set_line(int line_number = 0) {
  this->line_number = line_number;
}

void DebugScreen::output() {
  if(!debugMode) { return; }  
  
  display->setCursor(0, LINE_HEIGHT * line_number);
  display->setTextColor(BLACK);
  for(int i=0; i<MAX_LINE_LEN; i++) {
    display->write(FULL_CARET);
  }
  display->display();

  display->setCursor(0, LINE_HEIGHT * line_number);
  display->setTextColor(WHITE);
  display->print(display_messages[line_number]);
  display->display();  
}

void DebugScreen::say(String message, bool out=false, bool clear = false) {
  if(!debugMode) { return; }
  if(clear) { display_messages[line_number] = ""; }
  display_messages[line_number].concat(message);
  if(out) { output(); }
}

void DebugScreen::say(char* message, bool out=false, bool clear = false) {
  say(String(message), out, clear);
}

void DebugScreen::say(int integer, bool out=false, bool clear = false) {
  say(String(integer), out, clear);
}

void DebugScreen::say(float digit, bool out=false, bool clear = false) {
  say(String(digit), out, clear);
}

void DebugScreen::say(ArduinoJson::JsonArray& arr, bool out=false, bool clear = false) {
  if(!debugMode) { return; }
  char line_buffer[MAX_LINE_LEN];
  arr.printTo(line_buffer);
  say(String(line_buffer), out, clear);
}

void DebugScreen::say(String message) {
  say(message, false, false);
}

void DebugScreen::say(char* message) {
  say(message, false, false);
}

void DebugScreen::say(int integer) {
  say(integer, false, false);
}

void DebugScreen::say(float digit) {
  say(digit, false, false);
}

void DebugScreen::say(ArduinoJson::JsonArray& arr) {  
  say(arr, false, false);
}

void DebugScreen::sayn(String message) {
  say(message, false, true);
}

void DebugScreen::sayn(char* message) {
  say(message, false, true);
}

void DebugScreen::sayn(int integer) {
  say(integer, false, true);
}

void DebugScreen::sayn(float digit) {
  say(digit, false, true);
}

void DebugScreen::sayn(ArduinoJson::JsonArray& arr) {
  say(arr, false, true);
}

void DebugScreen::sayln(String message) {
  say(message, true, false);
}

void DebugScreen::sayln(char* message) {
  say(message, true, false);
}

void DebugScreen::sayln(int integer) {
  say(integer, true, false);
}

void DebugScreen::sayln(float digit) {
  say(digit, true, false);
}

void DebugScreen::sayln(ArduinoJson::JsonArray& arr) {
  say(arr, true, false);
}