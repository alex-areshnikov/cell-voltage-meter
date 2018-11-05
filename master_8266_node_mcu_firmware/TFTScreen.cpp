#include <ArduinoJson.h>
#include "TFTScreen.h"

TFTScreen::TFTScreen(bool tftEnabled, int8_t cs, int8_t rst, int8_t dc) {
  this->tftEnabled = tftEnabled;  
  
  if(tftEnabled) {
    this->tft = new Adafruit_ST7735(cs, rst, dc);
  }
}

void TFTScreen::initialize() {
  if(tftEnabled) {
    tft->initR(INITR_BLACKTAB);
    //tft.setTextWrap(false); // Allow text to run off right edge
    tft->fillScreen(ST77XX_BLACK);
    tft->setRotation(1);
  }
}

void TFTScreen::sayTotal(ArduinoJson::JsonArray& banks_voltages, int banks_count, int cell_count) {
  if(tftEnabled) {
    tft->setCursor(0, 0);
    tft->setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
    tft->setTextSize(1);

    for(int bank_number = 0; bank_number < banks_count; bank_number++) {
      tft->println(banks_voltages[bank_number]["name"].as<String>());

      for(int cell = 0; cell < cell_count; cell++) {
        char cell_buf[4];

        if(cell == cell_count/2) {
           tft->println();
        }

        dtostrf(banks_voltages[bank_number]["voltages"][cell].as<float>(), 4, 2, cell_buf);
        tft->print(cell_buf);
        tft->print("v ");
      }

      tft->println("\n");
    }
  }
}

