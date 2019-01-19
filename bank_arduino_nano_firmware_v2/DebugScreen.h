#include <ArduinoJson.h>
#include <Adafruit_SSD1306.h>

#define MAX_LINE_LEN 21
#define LINE_HEIGHT 8
#define OLED_RESET 4
#define FULL_CARET 218

class DebugScreen {
private:
    bool debugMode;
    int line_number;
    String display_messages[4];
    Adafruit_SSD1306 *display;

public:
    DebugScreen(bool);

    void initialize();

    void set_line(int);
    void output();

    void say(String, bool, bool);
    void say(char*, bool, bool);
    void say(int, bool, bool);
    void say(float, bool, bool);
    void say(ArduinoJson::JsonArray&, bool, bool);

    void say(String);
    void say(char*);
    void say(int);
    void say(float);
    void say(ArduinoJson::JsonArray&);

    void sayn(String);
    void sayn(char*);
    void sayn(int);
    void sayn(float);
    void sayn(ArduinoJson::JsonArray&);

    void sayln(String);
    void sayln(char*);
    void sayln(int);
    void sayln(float);
    void sayln(ArduinoJson::JsonArray&);
}; 