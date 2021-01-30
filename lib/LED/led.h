#ifndef LED_H
#define LED_H

class LED {
   public:
    static void setupPIN(int ledPin);
    static void turnON(int ledPin);
    static void turnOff(int ledPin);
};


#endif
