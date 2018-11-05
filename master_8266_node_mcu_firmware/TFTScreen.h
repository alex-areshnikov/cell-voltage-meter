#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <ArduinoJson.h>

class TFTScreen {
  private:
  bool tftEnabled;
  Adafruit_ST7735 *tft;

  public:
  TFTScreen(bool tftEnabled, int8_t cs, int8_t rst, int8_t dc);
  void initialize();
  void sayTotal(ArduinoJson::JsonArray& arr, int, int);
};