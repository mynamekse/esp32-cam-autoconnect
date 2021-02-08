#include "PIR.h"
#include "MQTTEvent.h"
int PIR::pin;
int PIR::value;
int PIR::state;
Call_back PIR::cb_detect;

void PIR::setup() {
    pinMode(PIR::pin, INPUT);
    PIR::value = 0;
    PIR::state = LOW;
}

void PIR::loop() {
    PIR::value = digitalRead(PIR::pin);
    // Serial.println(PIR::value);
    if (PIR::value == HIGH) {
        delay(100);  // delay 100 milliseconds

        if (PIR::state == LOW) {
            Serial.println((String)"Motion detected! " + millis());
            PIR::cb_detect();
         
            PIR::state = HIGH;  // update variable state to HIGH
        }
    } else {
        delay(200);  // delay 200 milliseconds

        if (PIR::state == HIGH) { 
            Serial.println("Motion stopped!");
            MQTTEvent::onCallBackPIRNotDetected();
            PIR::state = LOW;  // update variable state to LOW
        }
    }
}

void PIR::setOnDetact(Call_back c) {
    PIR::cb_detect = c;
}