#include <led.h>
#include <Arduino.h>

void  LED::setupPIN(int ledPin) 
{
    pinMode(ledPin,OUTPUT);
}

void LED::turnON(int ledPin) 
{
    digitalWrite(ledPin, HIGH);
}

void LED::turnOff(int ledPin) 
{
     digitalWrite(ledPin, LOW);
}
