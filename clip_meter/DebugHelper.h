class DebugHelper {
private:
    bool debugMode;

public:
    DebugHelper(bool debugMode);

    void initialize(int baud);

    void say(String message);
    void say(char* message);
    void say(int integer);

    void sayln(String message);
    void sayln(char* message);
    void sayln(int integer);
}; 