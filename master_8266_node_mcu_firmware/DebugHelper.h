#include <ArduinoJson.h>

class DebugHelper {
private:
    bool debugMode;

public:
    DebugHelper(bool debugMode);

    void initialize();

    void say(char* message);
    void say(int integer);
    void say(ArduinoJson::JsonArray& arr);

    void sayln(char* message);
    void sayln(int integer);
    void sayln(ArduinoJson::JsonArray& arr);
}; 