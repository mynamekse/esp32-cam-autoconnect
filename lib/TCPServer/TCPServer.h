#ifndef TCPserver_h
#define TCPserver_h
#include <ArduinoJson.h>
#include <WiFi.h>
class TCPserver {
   public:
    static WiFiServer server;
    static void timeLoop(unsigned long startMillis, unsigned long interval);
    static void setup();
    static void onHandleMessage(StaticJsonDocument<256> message);

    static void loop();
};



#endif
