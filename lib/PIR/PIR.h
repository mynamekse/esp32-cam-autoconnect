#ifndef PIR_h
#define PIR_h
#include <ArduinoJson.h>
#include <WiFi.h>
typedef void (*Call_back)();

class PIR {
   public:
    static int pin;
    static int state;
    static int value;
    static bool enable;
    static bool work;
    static void setup();
    static void setOnDetact(Call_back cb_detect);
    static Call_back cb_detect;
    static void loop();
};

#endif
