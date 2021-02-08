#include <Arduino.h>
#include <led.h>
bool LED::ledState;
void LED::setupPIN(int ledPin) {
    pinMode(ledPin, OUTPUT);
}

void LED::turnON(int ledPin) {
    digitalWrite(ledPin, HIGH);
    LED::ledState = true;
}

void LED::turnOff(int ledPin) {
    digitalWrite(ledPin, LOW);
    LED::ledState = false;
}

bool LED::getStatus() {
    return LED::ledState;
}
