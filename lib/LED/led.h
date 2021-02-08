#ifndef LED_H
#define LED_H

class LED {
   public:
    static bool ledState;
    static void setupPIN(int ledPin);
    static void turnON(int ledPin);
    static void turnOff(int ledPin);
    static bool getStatus();
};

#endif
