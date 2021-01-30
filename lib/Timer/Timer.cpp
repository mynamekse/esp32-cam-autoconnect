#include "Timer.h"
#include <Arduino.h>
void Timer::timeLoop(unsigned long startMillis, unsigned long interval) 
{
    while (millis() - startMillis <interval)
    {
       
    }
    
}
