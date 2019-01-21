#include <ArduinoJson.h>

class DebugHelper {
private:
    bool debugMode;

public:
    DebugHelper(bool debugMode);

    void initialize(int baud);

    void say(String message);
    void say(char* message);
    void say(int integer);
    void say(float fl);
    void say(ArduinoJson::JsonArray& arr);

    void sayln(String message);
    void sayln(char* message);
    void sayln(int integer);
    void sayln(float fl);
    void sayln(ArduinoJson::JsonArray& arr);
}; 