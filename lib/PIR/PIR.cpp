#include "PIR.h"

#include "MQTTEvent.h"
#include "Timer.h"
int PIR::pin;
int PIR::value;
int PIR::state;
bool PIR::enable;
bool PIR::work;
Call_back PIR::cb_detect;

void PIR::setup() {
    pinMode(PIR::pin, INPUT);
    PIR::value = 0;
    PIR::state = LOW;
    PIR::enable = false;
    PIR::work=false;
}

void PIR::loop() {
    // Serial.println(PIR::value);
    if (PIR::enable == true && PIR::work==true) {
        PIR::value = digitalRead(PIR::pin);
        if (PIR::value == HIGH) {
            // delay(100);  // delay 100 milliseconds
            Timer::timeLoop(0, 100);
            if (PIR::state == LOW) {
                Serial.println((String) "Motion detected! " + millis());
                PIR::cb_detect();

                PIR::state = HIGH;  // update variable state to HIGH
            }
        } else {
            // delay(200);
            Timer::timeLoop(0, 200);
            if (PIR::state == HIGH) {
                Serial.println("Motion stopped!");
                MQTTEvent::onCallBackPIRNotDetected();
                PIR::state = LOW;  // update variable state to LOW
            }
        }
    }
}

void PIR::setOnDetact(Call_back c) {
    PIR::cb_detect = c;
}